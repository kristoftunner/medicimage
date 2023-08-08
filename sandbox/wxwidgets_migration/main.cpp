#include <wx/wxprec.h>
#include <wx/wx.h>
#include <wx/image.h>
#include <wx/window.h>
#include <chrono>

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

static bool ShouldExit()
{
  std::time_t result = std::time(nullptr);
  auto time = std::gmtime(&result);

  if ((time->tm_year > 123) || (time->tm_mon > 8))
    return true;
  else
    return false;
}
 
bool MyApp::OnInit()
{
  Logger::Init();
  wxInitAllImageHandlers();  
  MyFrame *frame = new MyFrame();
  frame->SetSize(0, 0, 1400, 900);
  frame->Show(true);
  return !ShouldExit();
}

} // namespace app