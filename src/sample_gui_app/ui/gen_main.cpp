///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct  8 2012)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "gen_main.h"

///////////////////////////////////////////////////////////////////////////
using namespace ui;

MainFrameBase::MainFrameBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer17;
	bSizer17 = new wxBoxSizer( wxVERTICAL );
	
	m_panel5 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer18;
	bSizer18 = new wxBoxSizer( wxVERTICAL );
	
	m_checkForUpdates = new wxButton( m_panel5, wxID_ANY, _("Check for updates"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer18->Add( m_checkForUpdates, 0, wxALL, 5 );
	
	
	m_panel5->SetSizer( bSizer18 );
	m_panel5->Layout();
	bSizer18->Fit( m_panel5 );
	bSizer17->Add( m_panel5, 1, wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer17 );
	this->Layout();
	
	this->Centre( wxBOTH );
	
	// Connect Events
	m_checkForUpdates->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MainFrameBase::checkForUpdatesOnButtonClick ), NULL, this );
}

MainFrameBase::~MainFrameBase()
{
	// Disconnect Events
	m_checkForUpdates->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MainFrameBase::checkForUpdatesOnButtonClick ), NULL, this );
	
}
