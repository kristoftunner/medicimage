#include "drawing/component_wrappers.h"
#include "component_wrappers.h"
#include "core/log.h"
#include "image_handling/image_editor.h"
#include <opencv2/imgproc.hpp>
namespace medicimage
{
Entity RectangleComponentWrapper::CreateRectangle(glm::vec2 firstPoint, glm::vec2 secondPoint,
                                                  DrawObjectType objectType)
{
    auto entity = Entity::CreateEntity(0, "rectangle");
    entity.GetComponent<CommonAttributesComponent>().temporary = objectType == DrawObjectType::TEMPORARY ? true : false;

    auto &transform = entity.GetComponent<TransformComponent>();
    auto &color = entity.AddComponent<ColorComponent>();
    auto &rectangle = entity.AddComponent<RectangleComponent>();
    auto &thickness = entity.AddComponent<ThicknessComponent>();

    auto tmpRect = cv::Rect2f(cv::Point2f{firstPoint.x, firstPoint.y}, cv::Point2f{secondPoint.x, secondPoint.y});
    rectangle.width = tmpRect.width;
    rectangle.height = tmpRect.height;

    glm::vec2 topleft{tmpRect.tl().x, tmpRect.tl().y};
    transform.translation = topleft;
    return entity;
}

void RectangleComponentWrapper::UpdateShapeAttributes()
{
    auto &rectangle = m_entity.GetComponent<RectangleComponent>();
    if (!m_entity.HasComponent<BoundingContourComponent>())
        m_entity.AddComponent<BoundingContourComponent>();
    if (!m_entity.HasComponent<PickPointsComponent>())
        m_entity.AddComponent<PickPointsComponent>();

    auto &boundingBox = m_entity.GetComponent<BoundingContourComponent>();
    auto &pickPoints = m_entity.GetComponent<PickPointsComponent>();

    boundingBox.cornerPoints = {
        {0, 0}, {rectangle.width, 0}, {rectangle.width, rectangle.height}, {0, rectangle.height}, {0, 0}};
    pickPoints.pickPoints = {glm::vec2{rectangle.width, rectangle.height} + glm::vec2{0, -rectangle.height / 2},
                             glm::vec2{rectangle.width, rectangle.height} + glm::vec2{-rectangle.width / 2, 0},
                             {0, rectangle.height / 2},
                             glm::vec2{rectangle.width / 2, 0}};
}

void RectangleComponentWrapper::OnPickPointDrag(glm::vec2 diff, int selectedPoint)
{
    switch (selectedPoint) // [TODO REFACTOR]: extract these out into functions
    {
    case static_cast<int>(RectanglePicPoints::RIGHT): {
        m_entity.GetComponent<RectangleComponent>().width += diff.x;
        break;
    }
    case static_cast<int>(RectanglePicPoints::BOTTOM): {
        m_entity.GetComponent<RectangleComponent>().height += diff.y;
        break;
    }
    case static_cast<int>(RectanglePicPoints::LEFT): {
        m_entity.GetComponent<RectangleComponent>().width += -diff.x;
        m_entity.GetComponent<TransformComponent>().translation += glm::vec2{diff.x, 0};
        break;
    }
    case static_cast<int>(RectanglePicPoints::TOP): {
        m_entity.GetComponent<RectangleComponent>().height += -diff.y;
        m_entity.GetComponent<TransformComponent>().translation += glm::vec2{0, diff.y};
        break;
    }
    default:
        APP_CORE_ERR("Wrong pickpoint index({}) at component:{}", selectedPoint,
                     m_entity.GetComponent<IDComponent>().ID);
        break;
    }
    UpdateShapeAttributes();
}

void RectangleComponentWrapper::OnObjectDrag(glm::vec2 diff)
{
    // TODO: some checks wether we drag the component outside or not etc..
    auto &transform = m_entity.GetComponent<TransformComponent>();
    transform.translation += diff;
}

void RectangleComponentWrapper::Draw()
{
    auto &transform = m_entity.GetComponent<TransformComponent>();
    auto &commonAttributes = m_entity.GetComponent<CommonAttributesComponent>();
    auto &rectangle = m_entity.GetComponent<RectangleComponent>();
    auto &thickness = m_entity.GetComponent<ThicknessComponent>();
    auto &color = m_entity.GetComponent<ColorComponent>().color;
    auto &topleft = transform.translation;
    auto bottomright = topleft + glm::vec2{rectangle.width, rectangle.height};
    ImageEditor::DrawRectangle(topleft, bottomright, color, thickness.thickness, commonAttributes.filled);

    if (commonAttributes.selected)
    {
        auto &pickPoints = m_entity.GetComponent<PickPointsComponent>().pickPoints;
        for (auto &point : pickPoints)
        {
            ImageEditor::DrawCircle(point + transform.translation, s_pickPointBoxSize / 2, s_pickPointColor, 2, true);
        }
    }
}

Entity medicimage::CircleComponentWrapper::CreateCircle(glm::vec2 firstPoint, glm::vec2 secondPoint, float aspectRatio,
                                                        DrawObjectType objectType)
{
    auto entity = Entity::CreateEntity(0, "Circle");
    entity.GetComponent<CommonAttributesComponent>().temporary = objectType == DrawObjectType::TEMPORARY ? true : false;

    auto &transform = entity.GetComponent<TransformComponent>();
    transform.translation = firstPoint;

    auto &color = entity.AddComponent<ColorComponent>();
    auto &circle = entity.AddComponent<CircleComponent>();
    auto &thickness = entity.AddComponent<ThicknessComponent>();
    circle.radius = glm::length(firstPoint - secondPoint);
    circle.aspectRatio = aspectRatio;

    return entity;
}

void CircleComponentWrapper::UpdateShapeAttributes()
{
    auto &circle = m_entity.GetComponent<CircleComponent>();

    if (!m_entity.HasComponent<BoundingContourComponent>())
        m_entity.AddComponent<BoundingContourComponent>();
    if (!m_entity.HasComponent<PickPointsComponent>())
        m_entity.AddComponent<PickPointsComponent>();

    auto &boundingBox = m_entity.GetComponent<BoundingContourComponent>();
    auto &pickPoints = m_entity.GetComponent<PickPointsComponent>();
    // bounding polyigon of the circle is a square actually
    float aspectRatio = circle.aspectRatio;
    float radius = circle.radius;
    // need to scale the y component, because the aspect ratio is not 1:1
    boundingBox.cornerPoints = {{-radius, -radius * aspectRatio},
                                {radius, -radius * aspectRatio},
                                {radius, radius * aspectRatio},
                                {-radius, radius * aspectRatio},
                                {0, 0}};
    pickPoints.pickPoints = {{radius, 0}, {0, radius * aspectRatio}, {-radius, 0}, {0, -radius * aspectRatio}};
}

void CircleComponentWrapper::OnPickPointDrag(glm::vec2 diff, int selectedPoint)
{
    float &radius = m_entity.GetComponent<CircleComponent>().radius;
    switch (selectedPoint) // [TODO REFACTOR]: extract these out into functions
    {
    case static_cast<int>(CirclePickPoints::RIGHT): {
        auto updatedRadius = radius + diff.x;
        radius = updatedRadius > 0 ? updatedRadius : radius;
        break;
    }
    case static_cast<int>(CirclePickPoints::LEFT): {
        auto updatedRadius = radius - diff.x;
        radius = updatedRadius > 0 ? updatedRadius : radius;
        break;
    }
    case static_cast<int>(CirclePickPoints::BOTTOM): {
        auto updatedRadius = radius + diff.y;
        radius = updatedRadius > 0 ? updatedRadius : radius;
        break;
    }
    case static_cast<int>(CirclePickPoints::TOP): {
        auto updatedRadius = radius - diff.y;
        radius = updatedRadius > 0 ? updatedRadius : radius;
        break;
    }
    default:
        APP_CORE_ERR("Wrong pickpoint index({}) at component:{}", selectedPoint,
                     m_entity.GetComponent<IDComponent>().ID);
        break;
    }
    UpdateShapeAttributes();
}

void CircleComponentWrapper::OnObjectDrag(glm::vec2 diff)
{
    // TODO: some checks wether we drag the component outside or not etc..
    auto &transform = m_entity.GetComponent<TransformComponent>();
    transform.translation += diff;
}

void CircleComponentWrapper::Draw()
{
    auto &transform = m_entity.GetComponent<TransformComponent>();
    auto &commonAttributes = m_entity.GetComponent<CommonAttributesComponent>();
    auto &circle = m_entity.GetComponent<CircleComponent>();
    auto &color = m_entity.GetComponent<ColorComponent>().color;
    auto &thickness = m_entity.GetComponent<ThicknessComponent>();
    auto &center = transform.translation;
    ImageEditor::DrawCircle(center, circle.radius, color, thickness.thickness, commonAttributes.filled);

    if (commonAttributes.selected)
    {
        auto &pickPoints = m_entity.GetComponent<PickPointsComponent>().pickPoints;
        auto &translation = m_entity.GetComponent<TransformComponent>().translation;
        for (auto &point : pickPoints)
        {

            ImageEditor::DrawCircle(point + translation, s_pickPointBoxSize / 2, s_pickPointColor, 2, true);
        }
    }
}

Entity ArrowComponentWrapper::CreateArrow(glm::vec2 firstPoint, glm::vec2 secondPoint, DrawObjectType objectType)
{
    auto entity = Entity::CreateEntity(0, "Arrow");
    entity.GetComponent<CommonAttributesComponent>().temporary = objectType == DrawObjectType::TEMPORARY ? true : false;

    auto &transform = entity.GetComponent<TransformComponent>();
    transform.translation = firstPoint;

    auto &color = entity.AddComponent<ColorComponent>();
    auto &arrow = entity.AddComponent<ArrowComponent>();
    auto &thickness = entity.AddComponent<ThicknessComponent>();
    arrow.end = secondPoint - firstPoint;

    return entity;
}

void ArrowComponentWrapper::UpdateShapeAttributes()
{
    auto &arrow = m_entity.GetComponent<ArrowComponent>();

    if (!m_entity.HasComponent<BoundingContourComponent>())
        m_entity.AddComponent<BoundingContourComponent>();
    if (!m_entity.HasComponent<PickPointsComponent>())
        m_entity.AddComponent<PickPointsComponent>();
    auto &boundingBox = m_entity.GetComponent<BoundingContourComponent>();
    auto &pickPoints = m_entity.GetComponent<PickPointsComponent>();

    glm::vec2 vec = arrow.begin - arrow.end;
    glm::vec2 perp = glm::normalize(glm::vec2{-vec.y, vec.x});
    glm::vec2 offset = perp * glm::vec2(0.01f);
    boundingBox.cornerPoints = {offset, arrow.end + offset, arrow.end - offset, -offset, offset};
    pickPoints.pickPoints = {arrow.begin, arrow.end};
}

void ArrowComponentWrapper::OnPickPointDrag(glm::vec2 diff, int selectedPoint)
{
    switch (selectedPoint) // [TODO REFACTOR]: extract these out into functions
    {
    case static_cast<int>(ArrowPickPoints::BEGIN):
        m_entity.GetComponent<ArrowComponent>().begin += diff;
        break;
    case static_cast<int>(ArrowPickPoints::END):
        m_entity.GetComponent<ArrowComponent>().end += diff;
        break;
    default:
        APP_CORE_ERR("Wrong pickpoint index({}) at component:{}", selectedPoint,
                     m_entity.GetComponent<IDComponent>().ID);
        break;
    }
    UpdateShapeAttributes();
}

void ArrowComponentWrapper::OnObjectDrag(glm::vec2 diff)
{
    // TODO: some checks wether we drag the component outside or not etc..
    auto &transform = m_entity.GetComponent<TransformComponent>();
    transform.translation += diff;
}

void ArrowComponentWrapper::Draw()
{
    auto &transform = m_entity.GetComponent<TransformComponent>();
    auto &commonAttributes = m_entity.GetComponent<CommonAttributesComponent>();
    auto &arrow = m_entity.GetComponent<ArrowComponent>();
    auto &color = m_entity.GetComponent<ColorComponent>().color;
    auto &thickness = m_entity.GetComponent<ThicknessComponent>();
    auto begin = arrow.begin + transform.translation;
    auto end = arrow.end + transform.translation;
    ImageEditor::DrawArrow(begin, end, color, thickness.thickness);

    if (commonAttributes.selected)
    {
        auto &pickPoints = m_entity.GetComponent<PickPointsComponent>().pickPoints;
        for (auto &point : pickPoints)
        {
            ImageEditor::DrawCircle(point + transform.translation, s_pickPointBoxSize / 2, s_pickPointColor, 2, true);
        }
    }
}

Entity LineComponentWrapper::CreateLine(glm::vec2 firstPoint, glm::vec2 secondPoint, DrawObjectType objectType)
{
    auto entity = Entity::CreateEntity(0, "Line");
    entity.GetComponent<CommonAttributesComponent>().temporary = objectType == DrawObjectType::TEMPORARY ? true : false;

    auto &transform = entity.GetComponent<TransformComponent>();
    transform.translation = firstPoint;

    auto &color = entity.AddComponent<ColorComponent>();
    auto &line = entity.AddComponent<LineComponent>();
    auto &thickness = entity.AddComponent<ThicknessComponent>();
    line.end = secondPoint - firstPoint;

    return entity;
}

void LineComponentWrapper::UpdateShapeAttributes()
{
    auto &line = m_entity.GetComponent<LineComponent>();

    if (!m_entity.HasComponent<BoundingContourComponent>())
        m_entity.AddComponent<BoundingContourComponent>();
    if (!m_entity.HasComponent<PickPointsComponent>())
        m_entity.AddComponent<PickPointsComponent>();
    auto &boundingBox = m_entity.GetComponent<BoundingContourComponent>();
    auto &pickPoints = m_entity.GetComponent<PickPointsComponent>();

    glm::vec2 vec = line.begin - line.end;
    glm::vec2 perp = glm::normalize(glm::vec2{-vec.y, vec.x});
    glm::vec2 offset = perp * glm::vec2(0.01f);
    boundingBox.cornerPoints = {offset, line.end + offset, line.end - offset, -offset, offset};
    pickPoints.pickPoints = {line.begin, line.end};
}

void LineComponentWrapper::OnPickPointDrag(glm::vec2 diff, int selectedPoint)
{
    switch (selectedPoint) // [TODO REFACTOR]: extract these out into functions
    {
    case static_cast<int>(LinePickPoints::BEGIN):
        m_entity.GetComponent<LineComponent>().begin += diff;
        break;
    case static_cast<int>(LinePickPoints::END):
        m_entity.GetComponent<LineComponent>().end += diff;
        break;
    default:
        APP_CORE_ERR("Wrong pickpoint index({}) at component:{}", selectedPoint,
                     m_entity.GetComponent<IDComponent>().ID);
        break;
    }
    UpdateShapeAttributes();
}

void LineComponentWrapper::OnObjectDrag(glm::vec2 diff)
{
    // TODO: some checks wether we drag the component outside or not etc..
    auto &transform = m_entity.GetComponent<TransformComponent>();
    transform.translation += diff;
}

void LineComponentWrapper::Draw()
{
    auto &transform = m_entity.GetComponent<TransformComponent>();
    auto &commonAttributes = m_entity.GetComponent<CommonAttributesComponent>();
    auto &line = m_entity.GetComponent<LineComponent>();
    auto &color = m_entity.GetComponent<ColorComponent>().color;
    auto &thickness = m_entity.GetComponent<ThicknessComponent>();
    auto begin = line.begin + transform.translation;
    auto end = line.end + transform.translation;
    ImageEditor::DrawLine(begin, end, color, thickness.thickness);

    if (commonAttributes.selected)
    {
        auto &pickPoints = m_entity.GetComponent<PickPointsComponent>().pickPoints;
        for (auto &point : pickPoints)
        {
            ImageEditor::DrawCircle(point + transform.translation, s_pickPointBoxSize / 2, s_pickPointColor, 2, true);
        }
    }
}

Entity TextComponentWrapper::CreateText(glm::vec2 firstPoint, const glm::vec2 sheetSize, const std::string &inputText,
                                        int fontSize, DrawObjectType objectType)
{
    auto entity = Entity::CreateEntity(0, "Text");
    entity.GetComponent<CommonAttributesComponent>().temporary = objectType == DrawObjectType::TEMPORARY ? true : false;

    auto &transform = entity.GetComponent<TransformComponent>();
    auto &thickness = entity.AddComponent<ThicknessComponent>();
    thickness.thickness = 2;
    transform.translation = firstPoint;

    auto &text = entity.AddComponent<TextComponent>();
    text.text = inputText;
    text.fontSize = fontSize;

    auto textSize = ImageEditor::GetTextBoundingBox(text.text, text.fontSize, 5.0); // TODO: add thickness component
    text.boxSize = textSize / sheetSize;
    return entity;
}

void TextComponentWrapper::UpdateShapeAttributes()
{
    auto &text = m_entity.GetComponent<TextComponent>();
    if (!m_entity.HasComponent<BoundingContourComponent>())
        m_entity.AddComponent<BoundingContourComponent>();
    // no need for pickpoints for text

    auto &boundingBox = m_entity.GetComponent<BoundingContourComponent>();
    boundingBox.cornerPoints = {
        {0, 0}, {text.boxSize.x, 0}, {text.boxSize.x, text.boxSize.y}, {0, text.boxSize.y}, {0, 0}};
}

void TextComponentWrapper::OnObjectDrag(glm::vec2 diff)
{
    // TODO: some checks wether we drag the component outside or not etc..
    auto &transform = m_entity.GetComponent<TransformComponent>();
    transform.translation += diff;
}

void TextComponentWrapper::Draw()
{
    auto &transform = m_entity.GetComponent<TransformComponent>();
    auto &commonAttributes = m_entity.GetComponent<CommonAttributesComponent>();
    auto &thickness = m_entity.GetComponent<ThicknessComponent>();
    auto &text = m_entity.GetComponent<TextComponent>();
    ImageEditor::DrawText(transform.translation, text.text, text.fontSize,
                          thickness.thickness); // TODO: add thickness component
    auto &boundingBox = m_entity.GetComponent<BoundingContourComponent>();
    for (auto &c : boundingBox.cornerPoints)
    {
        c += transform.translation;
    }
    ImageEditor::DrawRectangle(boundingBox.cornerPoints[0], boundingBox.cornerPoints[2], {0, 0, 0, 1.0}, 1, false);
    return;
}

Entity SplineComponentWrapper::CreateSpline(glm::vec2 begin, glm::vec2 middle, glm::vec2 end, DrawObjectType objectType)
{
    auto entity = Entity::CreateEntity(0, "Spline");
    entity.GetComponent<CommonAttributesComponent>().temporary = objectType == DrawObjectType::TEMPORARY ? true : false;

    auto &transform = entity.GetComponent<TransformComponent>();
    transform.translation = begin;

    auto &color = entity.AddComponent<ColorComponent>();
    auto &spline = entity.AddComponent<SplineComponent>();
    auto &thickness = entity.AddComponent<ThicknessComponent>();
    color.color = {0.0, 0.0, 0.0, 1.0};
    spline.begin = {0.0, 0.0};
    spline.middle = middle - begin;
    spline.end = end - begin;
    spline.lineCount = static_cast<int>(glm::length(end - begin) / 0.01);
    return entity;
}

void SplineComponentWrapper::Draw()
{
    auto &transform = m_entity.GetComponent<TransformComponent>();
    auto &commonAttributes = m_entity.GetComponent<CommonAttributesComponent>();
    auto &spline = m_entity.GetComponent<SplineComponent>();
    auto &color = m_entity.GetComponent<ColorComponent>().color;
    auto &thickness = m_entity.GetComponent<ThicknessComponent>();

    auto begin = transform.translation + spline.begin;
    auto middle = transform.translation + spline.middle;
    auto end = transform.translation + spline.end;
    ImageEditor::DrawSpline(begin, middle, end, spline.lineCount, color, thickness.thickness);
}

Entity SkinTemplateComponentWrapper::CreateSkinTemplate(glm::vec2 firstPoint, glm::vec2 secondPoint,
                                                        DrawObjectType objectType)
{
    auto entity = Entity::CreateEntity(0, "Skin template");

    entity.GetComponent<CommonAttributesComponent>().temporary = objectType == DrawObjectType::TEMPORARY ? true : false;

    auto &transform = entity.GetComponent<TransformComponent>();
    transform.translation = firstPoint;
    auto &skinTemplate = entity.AddComponent<SkinTemplateComponent>();
    auto &thickness = entity.AddComponent<ThicknessComponent>();
    auto &color = entity.AddComponent<ColorComponent>();

    static constexpr float minBoundingWidth = 0.1f;
    static constexpr float minBoundingHeight = 0.1f;
    auto diff = secondPoint - firstPoint;
    if (abs(diff.x) >= minBoundingWidth && abs(diff.y) >= minBoundingHeight)
    {
        auto boundingRectSize = glm::abs(diff);
        skinTemplate.boundingRectSize = glm::abs(diff);
        skinTemplate.leftHorSliceCount = s_defaultHorizontalCount;
        skinTemplate.rightHorSliceCount = s_defaultHorizontalCount;
        skinTemplate.vertSliceCount = s_defaultVerticalCount;

        skinTemplate.vertSliceWidthSpan = s_defaultVerticalWidthSpan;
        skinTemplate.leftHorSliceWidthSpan = (1 - s_defaultVerticalWidthSpan) / 2.0;
        skinTemplate.rightHorSliceWidthSpan = (1 - s_defaultVerticalWidthSpan) / 2.0;
        skinTemplate.leftHorSliceHeightSpan = s_defaultHorizontalHeightSpan;
        skinTemplate.rightHorSliceHeightSpan = s_defaultHorizontalHeightSpan;
        // check the vertical slice width and horizontal height
        auto verticalSliceWidth =
            skinTemplate.vertSliceWidthSpan * skinTemplate.boundingRectSize.x / s_defaultVerticalCount;
        auto horizontalSliceHeight =
            skinTemplate.leftHorSliceHeightSpan * skinTemplate.boundingRectSize.y / s_defaultHorizontalCount;
        auto horizontalSliceWidth = (1 - skinTemplate.vertSliceWidthSpan) / 2 * skinTemplate.boundingRectSize.x;
        assert(verticalSliceWidth > s_minimumSliceSize);
        assert(horizontalSliceHeight > s_minimumSliceSize);

        skinTemplate.drawable = true;
        GenerateSlices(entity);
    }
    return entity;
}

void SkinTemplateComponentWrapper::GenerateSlices(Entity entity)
{
    auto &transform = entity.GetComponent<TransformComponent>();
    auto &skinTemplate = entity.GetComponent<SkinTemplateComponent>();

    if (skinTemplate.drawable == true)
    {
        // clear the current rectangles, because we will generate the new ones according to the number of slices and the
        // vertical/horizontal sizes
        for (auto e : skinTemplate.leftHorizontalSlices)
        {
            Entity::DestroyEntity(Entity(e));
        }
        skinTemplate.leftHorizontalSlices.clear();
        for (auto e : skinTemplate.rightHorizontalSlices)
        {
            Entity::DestroyEntity(Entity(e));
        }
        skinTemplate.rightHorizontalSlices.clear();
        for (auto e : skinTemplate.verticalSlices)
        {
            Entity::DestroyEntity(Entity(e));
        }
        skinTemplate.verticalSlices.clear();

        for (auto e : skinTemplate.splines)
        {
            Entity::DestroyEntity(Entity(e));
        }
        skinTemplate.splines.clear();

        // check the sizes
        auto &color = entity.GetComponent<ColorComponent>();
        auto objectType = entity.GetComponent<CommonAttributesComponent>().temporary ? DrawObjectType::TEMPORARY
                                                                                     : DrawObjectType::PERMANENT;
        auto verticalStartPoint = transform.translation +
                                  glm::vec2{skinTemplate.boundingRectSize.x * skinTemplate.leftHorSliceWidthSpan, 0.0};
        auto leftHorStartPoint = transform.translation + glm::vec2{0.0, (1 - skinTemplate.leftHorSliceHeightSpan) / 2 *
                                                                            skinTemplate.boundingRectSize.y};
        auto rightHorStartPoint =
            transform.translation +
            glm::vec2{skinTemplate.boundingRectSize.x *
                          (skinTemplate.leftHorSliceWidthSpan + skinTemplate.vertSliceWidthSpan),
                      (1 - skinTemplate.rightHorSliceHeightSpan) / 2 * skinTemplate.boundingRectSize.y};
        for (auto i = 0; i < skinTemplate.vertSliceCount; i++)
        {
            glm::vec2 sliceSize{skinTemplate.vertSliceWidthSpan / skinTemplate.vertSliceCount *
                                    skinTemplate.boundingRectSize.x,
                                skinTemplate.boundingRectSize.y};

            glm::vec2 vertTopLeft = verticalStartPoint + glm::vec2{i * sliceSize.x, 0.0};
            glm::vec2 vertBottomRight = verticalStartPoint + glm::vec2{(i + 1) * sliceSize.x, sliceSize.y};
            auto rect = RectangleComponentWrapper::CreateRectangle(vertTopLeft, vertBottomRight, objectType);
            rect.GetComponent<ColorComponent>() = color;
            rect.GetComponent<CommonAttributesComponent>().composed = true;
            rect.GetComponent<ThicknessComponent>() = entity.GetComponent<ThicknessComponent>();
            skinTemplate.verticalSlices.push_back(rect.GetHandle());
        }
        for (auto i = 0; i < skinTemplate.leftHorSliceCount; i++)
        {
            float sliceWidth = skinTemplate.leftHorSliceWidthSpan * skinTemplate.boundingRectSize.x;
            float sliceHeight =
                skinTemplate.leftHorSliceHeightSpan / skinTemplate.leftHorSliceCount * skinTemplate.boundingRectSize.y;

            glm::vec2 horTopLeft = leftHorStartPoint + glm::vec2{0.0, i * sliceHeight};
            glm::vec2 horBottomRight = leftHorStartPoint + glm::vec2{sliceWidth, (i + 1) * sliceHeight};
            auto rect = RectangleComponentWrapper::CreateRectangle(horTopLeft, horBottomRight, objectType);
            rect.GetComponent<ColorComponent>() = color;
            rect.GetComponent<CommonAttributesComponent>().composed = true;
            rect.GetComponent<ThicknessComponent>() = entity.GetComponent<ThicknessComponent>();
            skinTemplate.leftHorizontalSlices.push_back(rect.GetHandle());
        }

        for (auto i = 0; i < skinTemplate.rightHorSliceCount; i++)
        {
            float sliceWidth = skinTemplate.rightHorSliceWidthSpan * skinTemplate.boundingRectSize.x;
            float sliceHeight = skinTemplate.rightHorSliceHeightSpan / skinTemplate.rightHorSliceCount *
                                skinTemplate.boundingRectSize.y;

            glm::vec2 horTopLeft = rightHorStartPoint + glm::vec2{0.0, i * sliceHeight};
            glm::vec2 horBottomRight = rightHorStartPoint + glm::vec2{sliceWidth, (i + 1) * sliceHeight};
            auto rect = RectangleComponentWrapper::CreateRectangle(horTopLeft, horBottomRight, objectType);
            rect.GetComponent<ColorComponent>() = color;
            rect.GetComponent<CommonAttributesComponent>().composed = true;
            rect.GetComponent<ThicknessComponent>() = entity.GetComponent<ThicknessComponent>();
            skinTemplate.rightHorizontalSlices.push_back(rect.GetHandle());
        }

        if (skinTemplate.drawSpline)
        {
            // generate the slices
            auto boundingRectSize = skinTemplate.boundingRectSize;
            std::vector<glm::vec2> splinePoints = {
                glm::vec2{0, boundingRectSize.y / 2} + glm::vec2(transform.translation),
                glm::vec2{boundingRectSize.x / 2, 0} + glm::vec2(transform.translation),
                glm::vec2{boundingRectSize.x, boundingRectSize.y} + glm::vec2{0, -boundingRectSize.y / 2} +
                    glm::vec2(transform.translation),
                glm::vec2{boundingRectSize.x, boundingRectSize.y} + glm::vec2{-boundingRectSize.x / 2, 0} +
                    glm::vec2(transform.translation)};

            auto createSpline = [](glm::vec2 begin, glm::vec2 middle, glm::vec2 end, glm::vec4 color,
                                   DrawObjectType type) -> Entity {
                auto spline = SplineComponentWrapper::CreateSpline(begin, middle, end, type);
                spline.GetComponent<ColorComponent>().color = {0.0, 0.0, 0.0, 1.0};
                spline.GetComponent<CommonAttributesComponent>().composed = true;
                return spline;
            };
            skinTemplate.splines.push_back(
                createSpline(splinePoints[0], splinePoints[1], splinePoints[2], color.color, objectType));
            skinTemplate.splines.push_back(
                createSpline(splinePoints[0], splinePoints[3], splinePoints[2], color.color, objectType));
        }
    }
}

void SkinTemplateComponentWrapper::SetLeftHorizontalWidthSpan(float span)
{
    auto &skinTemplate = m_entity.GetComponent<SkinTemplateComponent>();
    auto diff = span - skinTemplate.leftHorSliceWidthSpan;
    assert(diff + skinTemplate.leftHorSliceWidthSpan < s_maxHorizontalWidthSpan);
    assert(diff + skinTemplate.leftHorSliceWidthSpan > s_minHorizontalWidthSpan);
    assert(diff - skinTemplate.vertSliceWidthSpan < s_maxVerticalWidthSpan);
    assert(diff - skinTemplate.vertSliceWidthSpan > s_minVerticalWidthSpan);
    skinTemplate.leftHorSliceWidthSpan += diff;
    skinTemplate.vertSliceWidthSpan -= diff;
}

void SkinTemplateComponentWrapper::SetRightHorizontalWidthSpan(float span)
{
    auto &skinTemplate = m_entity.GetComponent<SkinTemplateComponent>();
    auto diff = span - skinTemplate.rightHorSliceWidthSpan;
    assert(diff + skinTemplate.rightHorSliceWidthSpan < s_maxHorizontalWidthSpan);
    assert(diff + skinTemplate.rightHorSliceWidthSpan > s_minHorizontalWidthSpan);
    assert(diff - skinTemplate.vertSliceWidthSpan < s_maxVerticalWidthSpan);
    assert(diff - skinTemplate.vertSliceWidthSpan > s_minVerticalWidthSpan);
    skinTemplate.rightHorSliceWidthSpan += diff;
    skinTemplate.vertSliceWidthSpan -= diff;
}

void SkinTemplateComponentWrapper::SetVertcialWidthSpan(float span)
{
    auto &skinTemplate = m_entity.GetComponent<SkinTemplateComponent>();
    auto diff = span - skinTemplate.vertSliceWidthSpan;
    assert(diff / 2.0 - skinTemplate.rightHorSliceWidthSpan < s_maxHorizontalWidthSpan);
    assert(diff / 2.0 - skinTemplate.rightHorSliceWidthSpan > s_minHorizontalWidthSpan);
    assert(diff / 2.0 - skinTemplate.leftHorSliceWidthSpan < s_maxHorizontalWidthSpan);
    assert(diff / 2.0 - skinTemplate.leftHorSliceWidthSpan > s_minHorizontalWidthSpan);
    assert(diff + skinTemplate.vertSliceWidthSpan < s_maxVerticalWidthSpan);
    assert(diff + skinTemplate.vertSliceWidthSpan > s_minVerticalWidthSpan);
    skinTemplate.rightHorSliceWidthSpan -= diff / 2.0;
    skinTemplate.leftHorSliceWidthSpan -= diff / 2.0;
    skinTemplate.vertSliceWidthSpan += diff;
}

glm::ivec2 SkinTemplateComponentWrapper::GetVerticalSliceCountBounds()
{
    auto &skinTemplate = m_entity.GetComponent<SkinTemplateComponent>();
    auto maxBound = (skinTemplate.vertSliceWidthSpan * skinTemplate.boundingRectSize.x) / s_minimumSliceSize;
    return glm::ivec2{1, static_cast<int>(maxBound)};
}

glm::ivec2 SkinTemplateComponentWrapper::GetLeftHorizontalSliceCountBounds()
{
    auto &skinTemplate = m_entity.GetComponent<SkinTemplateComponent>();
    auto maxBound = (skinTemplate.leftHorSliceHeightSpan * skinTemplate.boundingRectSize.y) / s_minimumSliceSize;
    return glm::ivec2{1, static_cast<int>(maxBound)};
}

glm::ivec2 SkinTemplateComponentWrapper::GetRightHorizontalSliceCountBounds()
{
    auto &skinTemplate = m_entity.GetComponent<SkinTemplateComponent>();
    auto maxBound = (skinTemplate.rightHorSliceHeightSpan * skinTemplate.boundingRectSize.y) / s_minimumSliceSize;
    return glm::ivec2{1, static_cast<int>(maxBound)};
}

void SkinTemplateComponentWrapper::UpdateShapeAttributes()
{
    auto &skinTemplate = m_entity.GetComponent<SkinTemplateComponent>();
    if (!m_entity.HasComponent<BoundingContourComponent>())
        m_entity.AddComponent<BoundingContourComponent>();
    if (!m_entity.HasComponent<PickPointsComponent>())
        m_entity.AddComponent<PickPointsComponent>();

    auto &boundingBox = m_entity.GetComponent<BoundingContourComponent>();
    auto &pickPoints = m_entity.GetComponent<PickPointsComponent>();

    auto boundingRectSize = skinTemplate.boundingRectSize;
    boundingBox.cornerPoints = {
        {0, 0}, {boundingRectSize.x, 0}, {boundingRectSize.x, boundingRectSize.y}, {0, boundingRectSize.y}, {0, 0}};
    auto leftHorizontalTop = glm::vec2{skinTemplate.leftHorSliceWidthSpan * skinTemplate.boundingRectSize.x / 2,
                                       (1 - skinTemplate.leftHorSliceHeightSpan) / 2 * skinTemplate.boundingRectSize.y};
    auto verticalLeft = glm::vec2{skinTemplate.leftHorSliceWidthSpan * skinTemplate.boundingRectSize.x,
                                  skinTemplate.boundingRectSize.y / 2.0};
    auto rightHorizontalTop =
        glm::vec2{(skinTemplate.leftHorSliceWidthSpan + skinTemplate.vertSliceWidthSpan +
                   skinTemplate.rightHorSliceWidthSpan / 2.0) *
                      skinTemplate.boundingRectSize.x,
                  (1 - skinTemplate.rightHorSliceHeightSpan) / 2 * skinTemplate.boundingRectSize.y};
    pickPoints.pickPoints = {
        glm::vec2{boundingRectSize.x, boundingRectSize.y} + glm::vec2{0, -boundingRectSize.y / 2}, // RIGHT
        glm::vec2{boundingRectSize.x, boundingRectSize.y} + glm::vec2{-boundingRectSize.x / 2, 0}, // BOTTOM
        {0, boundingRectSize.y / 2},                                                               // LEFT
        glm::vec2{boundingRectSize.x / 2, 0},                                                      // TOP
        leftHorizontalTop,                                                                         // LEFT_SLICES_TOP
        leftHorizontalTop +
            glm::vec2{0.0, skinTemplate.leftHorSliceHeightSpan * skinTemplate.boundingRectSize.y}, // LEFT_SLICES_BOTTOM
        verticalLeft, //  MIDDLE_SLICES_LEFT
        verticalLeft +
            glm::vec2{skinTemplate.vertSliceWidthSpan * skinTemplate.boundingRectSize.x, 0.0}, // MIDDLE_SLICES_RIGHT
        rightHorizontalTop,                                                                    // RIGHT_SLICES_TOP
        rightHorizontalTop + glm::vec2{0.0, skinTemplate.rightHorSliceHeightSpan *
                                                skinTemplate.boundingRectSize.y} // RIGHT_SLICES_BOTTOM
    };

    GenerateSlices(m_entity);
}

void SkinTemplateComponentWrapper::OnPickPointDrag(glm::vec2 diff, int selectedPoint)
{
    auto addHorizontalDiff = [](float diff, float &span, float rectSize, int &sliceCount) {
        auto relDiff = diff / rectSize;
        auto newSpan = relDiff + span;
        auto newSliceSize = (relDiff + span) / sliceCount * rectSize;
        if (newSpan < s_maxHorizontalHeightSpan && newSpan > s_minHorizontalHeightSpan &&
            newSliceSize > s_minimumSliceSize)
        {
            span += relDiff;
        }
    };

    switch (selectedPoint) // [TODO REFACTOR]: extract these out into functions
    {
    case static_cast<int>(SkinTemplatePickPoints::RIGHT): {
        auto &skinTemplate = m_entity.GetComponent<SkinTemplateComponent>();
        skinTemplate.boundingRectSize.x += diff.x;
        GenerateSlices(m_entity);
        break;
    }
    case static_cast<int>(SkinTemplatePickPoints::BOTTOM): {
        auto &skinTemplate = m_entity.GetComponent<SkinTemplateComponent>();
        skinTemplate.boundingRectSize.y += diff.y;
        GenerateSlices(m_entity);
        break;
    }
    case static_cast<int>(SkinTemplatePickPoints::LEFT): {
        auto &skinTemplate = m_entity.GetComponent<SkinTemplateComponent>();
        skinTemplate.boundingRectSize.x += -diff.x;
        m_entity.GetComponent<TransformComponent>().translation += glm::vec2{diff.x, 0};
        GenerateSlices(m_entity);
        break;
    }
    case static_cast<int>(SkinTemplatePickPoints::TOP): {
        auto &skinTemplate = m_entity.GetComponent<SkinTemplateComponent>();
        skinTemplate.boundingRectSize.y += -diff.y;
        m_entity.GetComponent<TransformComponent>().translation += glm::vec2{0, diff.y};
        GenerateSlices(m_entity);
        break;
    }
    case static_cast<int>(SkinTemplatePickPoints::LEFT_SLICES_TOP): { // TODO REFACTOR: use here lambdas
        auto &skinTemplate = m_entity.GetComponent<SkinTemplateComponent>();
        addHorizontalDiff(-diff.y, skinTemplate.leftHorSliceHeightSpan, skinTemplate.boundingRectSize.y,
                          skinTemplate.leftHorSliceCount);
        break;
    }
    case static_cast<int>(SkinTemplatePickPoints::LEFT_SLICES_BOTTOM): {
        auto &skinTemplate = m_entity.GetComponent<SkinTemplateComponent>();
        addHorizontalDiff(diff.y, skinTemplate.leftHorSliceHeightSpan, skinTemplate.boundingRectSize.y,
                          skinTemplate.leftHorSliceCount);
        break;
    }
    case static_cast<int>(SkinTemplatePickPoints::MIDDLE_SLICES_LEFT): {
        auto &skinTemplate = m_entity.GetComponent<SkinTemplateComponent>();
        auto relDiff = -diff.x / skinTemplate.boundingRectSize.x;
        auto newVerticalSpan = relDiff + skinTemplate.vertSliceWidthSpan;
        auto newLeftHorSpan = skinTemplate.leftHorSliceWidthSpan - relDiff;
        auto newLeftHorSliceSize = (skinTemplate.leftHorSliceWidthSpan - relDiff) / skinTemplate.leftHorSliceCount *
                                   skinTemplate.boundingRectSize.x;
        auto newVerticalSliceSize =
            (relDiff + skinTemplate.vertSliceWidthSpan) / skinTemplate.vertSliceCount * skinTemplate.boundingRectSize.x;
        if (newVerticalSpan < s_maxVerticalWidthSpan && newVerticalSpan > s_minVerticalWidthSpan &&
            newVerticalSliceSize > s_minimumSliceSize && newLeftHorSpan < s_maxHorizontalWidthSpan &&
            newLeftHorSpan > s_minHorizontalWidthSpan && newLeftHorSliceSize > s_minimumSliceSize)
        {
            skinTemplate.vertSliceWidthSpan += relDiff;
            skinTemplate.leftHorSliceWidthSpan -= relDiff;
        }
        break;
    }
    case static_cast<int>(SkinTemplatePickPoints::MIDDLE_SLICES_RIGHT): {
        auto &skinTemplate = m_entity.GetComponent<SkinTemplateComponent>();
        auto relDiff = diff.x / skinTemplate.boundingRectSize.x;
        auto newVerticalSpan = relDiff + skinTemplate.vertSliceWidthSpan;
        auto newRightHorSpan = skinTemplate.rightHorSliceWidthSpan - relDiff;
        auto newRightHorSliceSize = (skinTemplate.rightHorSliceWidthSpan - relDiff) / skinTemplate.rightHorSliceCount *
                                    skinTemplate.boundingRectSize.x;
        auto newVerticalSliceSize =
            (relDiff + skinTemplate.vertSliceWidthSpan) / skinTemplate.vertSliceCount * skinTemplate.boundingRectSize.x;
        if (newVerticalSpan < s_maxVerticalWidthSpan && newVerticalSpan > s_minVerticalWidthSpan &&
            newVerticalSliceSize > s_minimumSliceSize && newRightHorSpan < s_maxHorizontalWidthSpan &&
            newRightHorSpan > s_minHorizontalWidthSpan && newRightHorSliceSize > s_minimumSliceSize)
        {
            skinTemplate.vertSliceWidthSpan += relDiff;
            skinTemplate.rightHorSliceWidthSpan -= relDiff;
        }
        break;
    }
    case static_cast<int>(SkinTemplatePickPoints::RIGHT_SLICES_TOP): {
        auto &skinTemplate = m_entity.GetComponent<SkinTemplateComponent>();
        addHorizontalDiff(-diff.y, skinTemplate.rightHorSliceHeightSpan, skinTemplate.boundingRectSize.y,
                          skinTemplate.rightHorSliceCount);
        break;
    }
    case static_cast<int>(SkinTemplatePickPoints::RIGHT_SLICES_BOTTOM): {
        auto &skinTemplate = m_entity.GetComponent<SkinTemplateComponent>();
        addHorizontalDiff(diff.y, skinTemplate.rightHorSliceHeightSpan, skinTemplate.boundingRectSize.y,
                          skinTemplate.rightHorSliceCount);
        break;
    }
    default:
        APP_CORE_ERR("Wrong pickpoint index({}) at component:{}", selectedPoint,
                     m_entity.GetComponent<IDComponent>().ID);
        break;
    }
    UpdateShapeAttributes();
}

void SkinTemplateComponentWrapper::OnObjectDrag(glm::vec2 diff)
{
    // TODO: some checks wether we drag the component outside or not etc..
    auto &transform = m_entity.GetComponent<TransformComponent>();
    transform.translation += diff;
    UpdateShapeAttributes();
}

void SkinTemplateComponentWrapper::Draw()
{
    auto &skinTemplate = m_entity.GetComponent<SkinTemplateComponent>();
    for (auto e : skinTemplate.verticalSlices)
    { // TODO REFACTOR: make it a lambda
        Entity rect(e);
        RectangleComponentWrapper rw(rect);
        rw.Draw();
    }
    for (auto e : skinTemplate.leftHorizontalSlices)
    {
        Entity rect(e);
        RectangleComponentWrapper rw(rect);
        rw.Draw();
    }
    for (auto e : skinTemplate.rightHorizontalSlices)
    {
        Entity rect(e);
        RectangleComponentWrapper rw(rect);
        rw.Draw();
    }
    for (auto e : skinTemplate.rightHorizontalSlices)
    {
        Entity rect(e);
        RectangleComponentWrapper rw(rect);
        rw.Draw();
    }
    for (auto e : skinTemplate.splines)
    {
        Entity entity(e);
        SplineComponentWrapper sw(entity);
        sw.Draw();
    }

    auto &commonAttributes = m_entity.GetComponent<CommonAttributesComponent>();
    if (commonAttributes.selected)
    {
        auto &pickPoints = m_entity.GetComponent<PickPointsComponent>().pickPoints;
        auto &translation = m_entity.GetComponent<TransformComponent>().translation;
        for (auto &point : pickPoints)
        {

            ImageEditor::DrawCircle(point + translation, s_pickPointBoxSize / 2, s_pickPointColor, 2, true);
        }
    }
}

} // namespace medicimage
