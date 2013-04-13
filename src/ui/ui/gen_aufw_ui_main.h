///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct  8 2012)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __GEN_AUFW_UI_MAIN_H__
#define __GEN_AUFW_UI_MAIN_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/menu.h>
#include <wx/listctrl.h>
#include <wx/gauge.h>
#include <wx/button.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////

namespace aufw
{
	namespace ui
	{
		///////////////////////////////////////////////////////////////////////////////
		/// Class FoundUpdatesDialogBase
		///////////////////////////////////////////////////////////////////////////////
		class FoundUpdatesDialogBase : public wxDialog 
		{
			private:
			
			protected:
				wxPanel* m_headerPanel;
				wxStaticText* m_headerText;
				wxPanel* m_releaseNotesContainer;
				wxPanel* m_productsPanel;
				wxListCtrl* m_products;
				wxMenu* m_menu1;
				wxPanel* m_progressPanel;
				wxStaticText* m_staticText7;
				wxGauge* m_progressGauge;
				wxPanel* m_footerPanel;
				wxStaticText* m_appRestartText;
				wxPanel* m_updateButtonsPanel;
				wxButton* m_updateNow;
				wxButton* m_skipUpdate;
				wxButton* m_dontUpdate;
				wxPanel* m_installButtonsPanel;
				wxButton* m_installNow;
				wxButton* m_dontInstall;
				
				// Virtual event handlers, overide them in your derived class
				virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
				virtual void OnInitDialog( wxInitDialogEvent& event ) { event.Skip(); }
				virtual void productsOnListItemSelected( wxListEvent& event ) { event.Skip(); }
				virtual void moreInfoOnMenuSelection( wxCommandEvent& event ) { event.Skip(); }
				virtual void moreInfoOnUpdateUI( wxUpdateUIEvent& event ) { event.Skip(); }
				virtual void manualDownloadOnMenuSelection( wxCommandEvent& event ) { event.Skip(); }
				virtual void manualDownloadOnUpdateUI( wxUpdateUIEvent& event ) { event.Skip(); }
				virtual void updateNowOnButtonClick( wxCommandEvent& event ) { event.Skip(); }
				virtual void skipUpdateOnButtonClick( wxCommandEvent& event ) { event.Skip(); }
				virtual void dontUpdateOnButtonClick( wxCommandEvent& event ) { event.Skip(); }
				virtual void installNowOnButtonClick( wxCommandEvent& event ) { event.Skip(); }
				virtual void dontInstallOnButtonClick( wxCommandEvent& event ) { event.Skip(); }
				
			
			public:
				
				FoundUpdatesDialogBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Update"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 680,500 ), long style = wxCAPTION|wxCLOSE_BOX|wxDIALOG_NO_PARENT|wxMAXIMIZE_BOX|wxMINIMIZE_BOX|wxRESIZE_BORDER|wxSYSTEM_MENU ); 
				~FoundUpdatesDialogBase();
				
				void m_productsOnContextMenu( wxMouseEvent &event )
				{
					m_products->PopupMenu( m_menu1, event.GetPosition() );
				}
			
		};
		
	} // namespace ui
} // namespace aufw

#endif //__GEN_AUFW_UI_MAIN_H__
