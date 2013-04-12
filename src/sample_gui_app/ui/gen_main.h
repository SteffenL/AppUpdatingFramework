///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct  8 2012)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __GEN_MAIN_H__
#define __GEN_MAIN_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/button.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/frame.h>

///////////////////////////////////////////////////////////////////////////

namespace ui
{
	///////////////////////////////////////////////////////////////////////////////
	/// Class MainFrameBase
	///////////////////////////////////////////////////////////////////////////////
	class MainFrameBase : public wxFrame 
	{
		private:
		
		protected:
			wxPanel* m_panel5;
			wxButton* m_checkForUpdates;
			
			// Virtual event handlers, overide them in your derived class
			virtual void checkForUpdatesOnButtonClick( wxCommandEvent& event ) { event.Skip(); }
			
		
		public:
			
			MainFrameBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,300 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
			
			~MainFrameBase();
		
	};
	
} // namespace ui

#endif //__GEN_MAIN_H__
