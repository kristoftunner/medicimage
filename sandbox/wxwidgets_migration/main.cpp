// wxWidgets "Hello World" Program
 
// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>
#include <wx/wx.h>
#include <wx/image.h>

#include "canvas.h"
#include "frame.h"
namespace app
{
class MyApp : public wxApp
{
public:
    virtual bool OnInit();
};
 
wxIMPLEMENT_APP(MyApp);
 
bool MyApp::OnInit()
{
  wxInitAllImageHandlers();  
  MyFrame *frame = new MyFrame();
  frame->Show(true);
  return true;
}

} // namespace app