#include <Cesium/Components/CesiumTileExcluderComponent.h>
#include <Cesium/EBus/CartographicPolygonComponentBus.h>
#include <Cesium/Math/GeospatialHelper.h>
#include <Cesium/EBus/OriginShiftComponentBus.h>
#include "Cesium/EBus/TilesetExcluderBus.h"
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Component/TransformBus.h>
#include <LmbrCentral/Shape/PolygonPrismShapeComponentBus.h>

// Windows wingdi.h OPAQUE macro workaround
#include <AzCore/PlatformDef.h>
#ifdef AZ_COMPILER_MSVC
#pragma push_macro("OPAQUE")
#undef OPAQUE
#endif

#include <Cesium3DTilesSelection/ITileExcluder.h>
#include <Cesium3DTilesSelection/Tile.h>
#include <CesiumGeospatial/CartographicPolygon.h>
#include <CesiumGeospatial/Cartographic.h>
#include <CesiumGeospatial/BoundingRegion.h>
#include <CesiumGeospatial/GlobeRectangle.h>
#include <CesiumGeospatial/Ellipsoid.h>
#include <CesiumGeometry/OrientedBoundingBox.h>
#include <CesiumGeometry/BoundingSphere.h>

#ifdef AZ_COMPILER_MSVC
#pragma pop_macro("OPAQUE")
#endif

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace Cesium
{
    // ============================================================
    // Custom ITileExcluder: checks tile bounding regions against polygons
    // ============================================================
    class CartographicPolygonExcluder : public Cesium3DTilesSelection::ITileExcluder
    {
    public:
        struct PolygonBounds
        {
            PolygonBounds(
                CesiumGeospatial::CartographicPolygon polygon,
                CesiumGeospatial::GlobeRectangle bounds,
                bool invertSelection)
                : m_polygon(std::move(polygon))
                , m_bounds(bounds)
                , m_invertSelection(invertSelection)
            {
            }

            CesiumGeospatial::CartographicPolygon m_polygon;
            CesiumGeospatial::GlobeRectangle m_bounds;
            bool m_invertSelection = false;
        };

        void SetPolygons(AZStd::vector<PolygonBounds>&& polygons)
        {
            m_polygons = AZStd::move(polygons);
        }

        bool shouldExclude(const Cesium3DTilesSelection::Tile& tile) const noexcept override
        {
            if (m_polygons.empty())
            {
                return false;
            }

            // Extract tile center in cartographic (lon/lat radians)
            double centerLon = 0.0, centerLat = 0.0;
            bool hasCenter = false;

            const auto& bv = tile.getBoundingVolume();

            if (const auto* pRegion = std::get_if<CesiumGeospatial::BoundingRegion>(&bv))
            {
                const auto& rect = pRegion->getRectangle();
                centerLon = (rect.getWest() + rect.getEast()) * 0.5;
                centerLat = (rect.getSouth() + rect.getNorth()) * 0.5;
                hasCenter = true;
            }
            else if (const auto* pObb = std::get_if<CesiumGeometry::OrientedBoundingBox>(&bv))
            {
                // OBB center is in ECEF; convert to cartographic
                const glm::dvec3& ecefCenter = pObb->getCenter();
                auto cartOpt = CesiumGeospatial::Ellipsoid::WGS84.cartesianToCartographic(ecefCenter);
                if (cartOpt)
                {
                    centerLon = cartOpt->longitude;
                    centerLat = cartOpt->latitude;
                    hasCenter = true;
                }
            }
            else if (const auto* pSphere = std::get_if<CesiumGeometry::BoundingSphere>(&bv))
            {
                // Sphere center is in ECEF; convert to cartographic
                const glm::dvec3& ecefCenter = pSphere->getCenter();
                auto cartOpt = CesiumGeospatial::Ellipsoid::WGS84.cartesianToCartographic(ecefCenter);
                if (cartOpt)
                {
                    centerLon = cartOpt->longitude;
                    centerLat = cartOpt->latitude;
                    hasCenter = true;
                }
            }

            if (!hasCenter)
            {
                return false;
            }

            // Check tile center against each polygon
            for (const auto& polyBounds : m_polygons)
            {
                // Quick rejection: is center inside polygon's bounding rectangle?
                if (centerLon < polyBounds.m_bounds.getWest() || centerLon > polyBounds.m_bounds.getEast() ||
                    centerLat < polyBounds.m_bounds.getSouth() || centerLat > polyBounds.m_bounds.getNorth())
                {
                    continue;
                }

                // Center is within polygon's bounding box - do point-in-polygon test
                if (PointInPolygon(centerLon, centerLat, polyBounds.m_polygon))
                {
                    return !polyBounds.m_invertSelection;
                }
            }

            return false;
        }

    private:
        //! Ray-casting point-in-polygon test.
        static bool PointInPolygon(
            double lon, double lat,
            const CesiumGeospatial::CartographicPolygon& polygon)
        {
            const auto& vertices = polygon.getVertices();
            if (vertices.size() < 3)
            {
                return false;
            }

            bool inside = false;
            size_t n = vertices.size();
            for (size_t i = 0, j = n - 1; i < n; j = i++)
            {
                double xi = vertices[i].x, yi = vertices[i].y;
                double xj = vertices[j].x, yj = vertices[j].y;

                if (((yi > lat) != (yj > lat)) &&
                    (lon < (xj - xi) * (lat - yi) / (yj - yi) + xi))
                {
                    inside = !inside;
                }
            }
            return inside;
        }

        AZStd::vector<PolygonBounds> m_polygons;
    };

    // ============================================================
    // CesiumTileExcluderComponent::Impl
    // ============================================================
    struct CesiumTileExcluderComponent::Impl
    {
        std::shared_ptr<CartographicPolygonExcluder> m_excluder;
        bool m_excluderAdded = false;
        bool m_dirty = true;

        //! Directly read PolygonPrismShapeComponent on an entity and convert to lon/lat radians.
        static AZStd::vector<glm::dvec2> ReadPolygonPrismShapeAsRadians(AZ::EntityId entityId)
        {
            AZStd::vector<glm::dvec2> result;

            AZ::PolygonPrismPtr prismPtr;
            LmbrCentral::PolygonPrismShapeComponentRequestBus::EventResult(
                prismPtr, entityId,
                &LmbrCentral::PolygonPrismShapeComponentRequests::GetPolygonPrism);

            if (!prismPtr)
            {
                return result;
            }

            const auto& vertices2D = prismPtr->m_vertexContainer.GetVertices();
            if (vertices2D.size() < 3)
            {
                return result;
            }

            // Get entity world transform
            AZ::Transform worldTM = AZ::Transform::CreateIdentity();
            AZ::TransformBus::EventResult(worldTM, entityId, &AZ::TransformBus::Events::GetWorldTM);

            // Get O3DE -> ECEF transform
            glm::dmat4 relToAbsWorld{ 1.0 };
            OriginShiftRequestBus::BroadcastResult(relToAbsWorld, &OriginShiftRequestBus::Events::GetRelToAbsWorld);

            result.reserve(vertices2D.size());
            for (const auto& v2d : vertices2D)
            {
                AZ::Vector3 localPos(v2d.GetX(), v2d.GetY(), 0.0f);
                AZ::Vector3 worldPos = worldTM.TransformPoint(localPos);

                glm::dvec4 relPos(
                    static_cast<double>(worldPos.GetX()),
                    static_cast<double>(worldPos.GetY()),
                    static_cast<double>(worldPos.GetZ()),
                    1.0);
                glm::dvec4 ecefPos = relToAbsWorld * relPos;
                glm::dvec3 ecef(ecefPos.x, ecefPos.y, ecefPos.z);

                auto cartographic = GeospatialHelper::ECEFCartesianToCartographic(ecef);
                if (cartographic)
                {
                    // Return radians directly (CesiumNative convention)
                    result.emplace_back(cartographic->m_longitude, cartographic->m_latitude);
                }
            }

            return result;
        }

        void CollectPolygonsAndUpdate(
            const AZStd::vector<AZ::EntityId>& polygonEntityIds,
            AZ::EntityId tilesetEntityId)
        {
            AZStd::vector<CartographicPolygonExcluder::PolygonBounds> polygonBounds;

            for (const auto& entityId : polygonEntityIds)
            {
                AZStd::vector<glm::dvec2> pointsRadians;
                bool invertSelection = false;

                // First try: query CartographicPolygonComponent
                CartographicPolygonRequestBus::EventResult(
                    pointsRadians, entityId, &CartographicPolygonRequestBus::Events::GetPolygonPointsRadians);
                CartographicPolygonRequestBus::EventResult(
                    invertSelection, entityId, &CartographicPolygonRequestBus::Events::GetInvertSelection);

                // Fallback: if no CartographicPolygonComponent, read PolygonPrismShape directly
                if (pointsRadians.size() < 3)
                {
                    pointsRadians = ReadPolygonPrismShapeAsRadians(entityId);
                }

                if (pointsRadians.size() < 3)
                {
                    continue;
                }

                // Convert to CesiumGeospatial::CartographicPolygon
                std::vector<glm::dvec2> nativePoints(pointsRadians.begin(), pointsRadians.end());
                CesiumGeospatial::CartographicPolygon polygon(nativePoints);

                // Compute bounding rectangle
                double minLon = std::numeric_limits<double>::max();
                double maxLon = std::numeric_limits<double>::lowest();
                double minLat = std::numeric_limits<double>::max();
                double maxLat = std::numeric_limits<double>::lowest();
                for (const auto& pt : nativePoints)
                {
                    minLon = std::min(minLon, pt.x);
                    maxLon = std::max(maxLon, pt.x);
                    minLat = std::min(minLat, pt.y);
                    maxLat = std::max(maxLat, pt.y);
                }

                polygonBounds.emplace_back(
                    std::move(polygon),
                    CesiumGeospatial::GlobeRectangle(minLon, minLat, maxLon, maxLat),
                    invertSelection);
            }

            m_excluder->SetPolygons(std::move(polygonBounds));

            // Add excluder to tileset if not already added
            if (!m_excluderAdded)
            {
                TilesetExcluderBus::Event(
                    tilesetEntityId, &TilesetExcluderBus::Events::AddTileExcluder, m_excluder);
                m_excluderAdded = true;
            }
        }

        void RemoveExcluder(AZ::EntityId tilesetEntityId)
        {
            if (m_excluderAdded)
            {
                TilesetExcluderBus::Event(
                    tilesetEntityId, &TilesetExcluderBus::Events::RemoveTileExcluder, m_excluder);
                m_excluderAdded = false;
            }
        }
    };

    // ============================================================
    // CesiumTileExcluderComponent
    // ============================================================
    void CesiumTileExcluderComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto* sc = azrtti_cast<AZ::SerializeContext*>(context))
        {
            sc->Class<CesiumTileExcluderComponent, AZ::Component>()
                ->Version(0)
                ->Field("PolygonEntityIds", &CesiumTileExcluderComponent::m_polygonEntityIds)
                ->Field("Enabled", &CesiumTileExcluderComponent::m_enabled);

            if (auto* ec = sc->GetEditContext())
            {
                ec->Class<CesiumTileExcluderComponent>(
                        "Cesium Tile Excluder",
                        "Excludes (clips) tiles from a tileset based on cartographic polygon regions. "
                        "Use for 'dig hole' scenarios to place custom buildings inside 3D Tiles terrain.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Cesium")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &CesiumTileExcluderComponent::m_enabled,
                        "Enabled", "Enable tile exclusion")
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &CesiumTileExcluderComponent::m_polygonEntityIds,
                        "Polygon Entities", "Entities with Cesium Cartographic Polygon components");
            }
        }

        if (auto* bc = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            bc->Class<CesiumTileExcluderComponent>("CesiumTileExcluderComponent")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/Polygon");
        }
    }

    void CesiumTileExcluderComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    { provided.push_back(AZ_CRC_CE("CesiumTileExcluderService")); }

    void CesiumTileExcluderComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    { incompatible.push_back(AZ_CRC_CE("CesiumTileExcluderService")); }

    void CesiumTileExcluderComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    { }

    void CesiumTileExcluderComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        dependent.push_back(AZ_CRC_CE("3DTilesService"));
        dependent.push_back(AZ_CRC_CE("3DTilesEditorService"));
    }

    CesiumTileExcluderComponent::CesiumTileExcluderComponent() = default;

    CesiumTileExcluderComponent::~CesiumTileExcluderComponent() noexcept = default;

    void CesiumTileExcluderComponent::Init()
    {
        m_impl = AZStd::make_unique<Impl>();
        m_impl->m_excluder = std::make_shared<CartographicPolygonExcluder>();
    }

    void CesiumTileExcluderComponent::Activate()
    {
        m_impl->m_dirty = true;
        AZ::TickBus::Handler::BusConnect();
    }

    void CesiumTileExcluderComponent::SetPolygonEntityIds(const AZStd::vector<AZ::EntityId>& entityIds)
    {
        m_polygonEntityIds = entityIds;
        m_impl->m_dirty = true;
    }

    void CesiumTileExcluderComponent::SetEnabled(bool enabled)
    {
        m_enabled = enabled;
    }

    void CesiumTileExcluderComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
        m_impl->RemoveExcluder(GetEntityId());
    }

    void CesiumTileExcluderComponent::OnTick(
        [[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        if (!m_enabled)
        {
            m_impl->RemoveExcluder(GetEntityId());
            return;
        }

        // For now, update every frame. Could be optimized with dirty tracking.
        m_impl->CollectPolygonsAndUpdate(m_polygonEntityIds, GetEntityId());
    }
} // namespace Cesium
