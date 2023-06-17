#include <wx/wxprec.h>
#include <wx/wx.h>
#include <wx/image.h>

#include "core/log.h"
#include "gui/editor/canvas.h"
#include "gui/frame.h"

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
  frame->SetSize(0, 0, 800, 600);
  frame->Show(true);
  return true;
}

} // namespace app