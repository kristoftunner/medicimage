// wxWidgets "Hello World" Program
 
// For compilers that support precompilation, includes "wx/wx.h".

#include "canvas.h"
#include "frame.h"
#include <wx/wxprec.h>
#include "core/log.h"

#include <wx/wx.h>
#include <wx/image.h>
namespace app
{
using namespace medicimage;

class MyApp : public wxApp
{
public:
    virtual bool OnInit();
};
 
wxIMPLEMENT_APP(MyApp);
 
bool MyApp::OnInit()
{
  Logger::Init();
  wxInitAllImageHandlers();  
  MyFrame *frame = new MyFrame();
  frame->Show(true);
  return true;
}

} // namespace app