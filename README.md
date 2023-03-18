# medicimage

## Icons
For the icons I use this lib: https://www.flaticon.com/search?style_id=1219&author_id=392&word=button

## Dear ImGui related:
Input text using the enter key for completion: https://github.com/ocornut/imgui/issues/589

## Notes on the ECS integration and objects on the sheet
- Drawing object has to have the following components
  - Transform component:
    - with the attirbutes of translation, rotation and (scale??)
    - scale can have multiple DoF(degrees of freedom), in case of a rectangle: 2 - this should be in the geomtery component
  - Color component - rgba
  - Tag component:
    - basecally to name/tag the component
  - Bounding contour
  - Pick points for changing visually attributes of the component by dragging the pick points 
- These are optional components:
  - Group component - optional
    - to tag if the component is grouped
- Geomtery description components:
  - Circle
  - Rectangle
  - Text
  - Ellipse
  - Arrow

## Drawing sheet concept
To be cleared: how to handle mouse events correctly when editing the scene(sheet - have correct naming here). A solution could be to have a <code>DrawingSheet</code>
class that contains the entt integration and all the drawing objects, which pass these objects to the draw command(wrapper opencv draw functions).

### Drawing states for the various commands:
During editing the default state is object selection. If a draw command is selected then the initial draw state should be set
**Draw circle/rectangle/arrow/**  
1) Initial -- just hovering on the document -> Left click - 2)
2) Draw first point
3) Drawing second point(left button down)
4) Second point drawn (left button released)
5) Object selected. Left click anywhere outside the pick area-> Draw first point  

**Add text**  
1) Initial -- just hovering on the document
2) Left click on the document -> 3) 
3) Adding text -> Left clock on the document -> ObjectSelectionInitialState 

**Select object**
1) ObjectSelectionInitialState. Mouse pressed -> 2) 
2) ObjectSelectionState. Mouse down: updating the selection area. Mouse released: if objects selected -> 3) else -> 1)
3) ObjectSelectedState. Mouse press on pick point -> 4) Mouse press on drag area -> 5)  Mouse press outside -> clear selection, 1)
4) PickPointSelectedState. Mouse down -> change attribute of the component. Mouse relesead -> 3)
5) ObjectDraggingState. Mouse down -> change the translation of the component. Mouse released -> 3)

**Discussion with samer**
For line: draw multiple separate line continuously
For shape: after drawing an object -> select that object and go back to selection mode

## TODOs/questions
Store the document in an image - this way the objects that are drawn previously cannot be edited, because for that we need metadata

Store the document with all the metadata (drawn objects and it's attributes) - this is heavy, but could be a very useful feature

## Important features
### customizable things
- color of the instances should be changeable
- text 
- thickness of lines
- templates: 
  - skin template:
    - number of slices
    - number of 

## 2023.03.17. features update
- [x] color palette for the color edit (black, red, green, blue, white, magenta)
- [ ] skin template: checkbox for adding the ellipse(black)
- [x] skin template: span should be 90%
- [x] default color: red
- [x] skintemplate: vertical slices not centered and should be configured by pickpoints
- [ ] automatatically labeled boxes
- [ ] automaticaly incremented text labeling
- [x] Implement the textbox
- [ ] fix every bug
- [ ] properties for edit
- [x] Implement line tool, if polygon easy implement that one as well
