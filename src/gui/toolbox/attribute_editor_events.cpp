#include "attribute_editor_events.h"

namespace app
{ 

// event to be used by the attribute editor to notify the editor that an entity attribute has changed
wxDEFINE_EVENT(EVT_ENTITY_ATTRIBUTE_EDITED, EntityEvent);
wxDEFINE_EVENT(EVT_PATIENT_SELECTED, PatientSelectedEvent);
wxDEFINE_EVENT(EVT_PATIENT_UPDATED, UpdatePatientsEvent);

} // app