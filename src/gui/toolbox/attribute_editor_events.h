#pragma once

#include <wx/event.h>

#include "gui/custom_events.h"

namespace app
{

using namespace medicimage;

wxDECLARE_EVENT(EVT_ENTITY_ATTRIBUTE_EDITED, EntityEvent);
wxDECLARE_EVENT(EVT_PATIENT_SELECTED, PatientSelectedEvent);
wxDECLARE_EVENT(EVT_PATIENT_UPDATED, UpdatePatientsEvent);
  
} // namespace app
