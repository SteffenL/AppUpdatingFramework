///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct  8 2012)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "gen_aufw_ui_main.h"

///////////////////////////////////////////////////////////////////////////
using namespace aufw::ui;

FoundUpdatesDialogBase::FoundUpdatesDialogBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer89;
	bSizer89 = new wxBoxSizer( wxVERTICAL );
	
	m_headerPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer49;
	bSizer49 = new wxBoxSizer( wxVERTICAL );
	
	m_headerText = new wxStaticText( m_headerPanel, wxID_ANY, _("MyLabel"), wxDefaultPosition, wxDefaultSize, 0 );
	m_headerText->Wrap( -1 );
	bSizer49->Add( m_headerText, 0, wxALL, 5 );
	
	
	m_headerPanel->SetSizer( bSizer49 );
	m_headerPanel->Layout();
	bSizer49->Fit( m_headerPanel );
	bSizer89->Add( m_headerPanel, 0, wxEXPAND|wxALL, 5 );
	
	wxBoxSizer* bSizer12;
	bSizer12 = new wxBoxSizer( wxVERTICAL );
	
	m_releaseNotesContainer = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer9;
	bSizer9 = new wxBoxSizer( wxVERTICAL );
	
	
	m_releaseNotesContainer->SetSizer( bSizer9 );
	m_releaseNotesContainer->Layout();
	bSizer9->Fit( m_releaseNotesContainer );
	bSizer12->Add( m_releaseNotesContainer, 3, wxEXPAND|wxRIGHT|wxLEFT, 5 );
	
	m_productsPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer11;
	bSizer11 = new wxBoxSizer( wxVERTICAL );
	
	m_products = new wxListCtrl( m_productsPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL );
	m_menu1 = new wxMenu();
	wxMenuItem* m_moreInfo;
	m_moreInfo = new wxMenuItem( m_menu1, wxID_ANY, wxString( _("More info && Discuss") ) , wxEmptyString, wxITEM_NORMAL );
	m_menu1->Append( m_moreInfo );
	
	wxMenuItem* m_manualDownload;
	m_manualDownload = new wxMenuItem( m_menu1, wxID_ANY, wxString( _("Manual download") ) , wxEmptyString, wxITEM_NORMAL );
	m_menu1->Append( m_manualDownload );
	
	m_products->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( FoundUpdatesDialogBase::m_productsOnContextMenu ), NULL, this ); 
	
	bSizer11->Add( m_products, 1, wxEXPAND, 5 );
	
	
	m_productsPanel->SetSizer( bSizer11 );
	m_productsPanel->Layout();
	bSizer11->Fit( m_productsPanel );
	bSizer12->Add( m_productsPanel, 2, wxEXPAND|wxALL, 5 );
	
	
	bSizer89->Add( bSizer12, 1, wxEXPAND|wxRIGHT|wxLEFT, 5 );
	
	m_downloadProgressPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer13;
	bSizer13 = new wxBoxSizer( wxVERTICAL );
	
	m_staticText7 = new wxStaticText( m_downloadProgressPanel, wxID_ANY, _("Download progress"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText7->Wrap( -1 );
	bSizer13->Add( m_staticText7, 0, wxALL, 5 );
	
	m_downloadProgress = new wxGauge( m_downloadProgressPanel, wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL );
	m_downloadProgress->SetValue( 0 ); 
	bSizer13->Add( m_downloadProgress, 0, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );
	
	
	m_downloadProgressPanel->SetSizer( bSizer13 );
	m_downloadProgressPanel->Layout();
	bSizer13->Fit( m_downloadProgressPanel );
	bSizer89->Add( m_downloadProgressPanel, 0, wxEXPAND|wxTOP|wxRIGHT|wxLEFT, 5 );
	
	m_footerPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxVERTICAL );
	
	m_appRestartText = new wxStaticText( m_footerPanel, wxID_ANY, _("MyLabel"), wxDefaultPosition, wxDefaultSize, 0 );
	m_appRestartText->Wrap( -1 );
	bSizer10->Add( m_appRestartText, 0, wxALL, 5 );
	
	
	m_footerPanel->SetSizer( bSizer10 );
	m_footerPanel->Layout();
	bSizer10->Fit( m_footerPanel );
	bSizer89->Add( m_footerPanel, 0, wxEXPAND|wxRIGHT|wxLEFT, 5 );
	
	m_updateButtonsPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer91;
	bSizer91 = new wxBoxSizer( wxHORIZONTAL );
	
	m_updateNow = new wxButton( m_updateButtonsPanel, wxID_ANY, _("Update now"), wxDefaultPosition, wxDefaultSize, 0 );
	m_updateNow->SetDefault(); 
	m_updateNow->SetMinSize( wxSize( 120,-1 ) );
	
	bSizer91->Add( m_updateNow, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	
	bSizer91->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_skipUpdate = new wxButton( m_updateButtonsPanel, wxID_ANY, _("Skip this one"), wxDefaultPosition, wxDefaultSize, 0 );
	m_skipUpdate->Hide();
	m_skipUpdate->SetMinSize( wxSize( 120,-1 ) );
	
	bSizer91->Add( m_skipUpdate, 0, wxTOP|wxLEFT, 5 );
	
	m_dontUpdate = new wxButton( m_updateButtonsPanel, wxID_ANY, _("Don't update now"), wxDefaultPosition, wxDefaultSize, 0 );
	m_dontUpdate->SetMinSize( wxSize( 120,-1 ) );
	
	bSizer91->Add( m_dontUpdate, 0, wxALL, 5 );
	
	
	m_updateButtonsPanel->SetSizer( bSizer91 );
	m_updateButtonsPanel->Layout();
	bSizer91->Fit( m_updateButtonsPanel );
	bSizer89->Add( m_updateButtonsPanel, 0, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );
	
	m_installButtonsPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer121;
	bSizer121 = new wxBoxSizer( wxHORIZONTAL );
	
	m_installNow = new wxButton( m_installButtonsPanel, wxID_ANY, _("Install now"), wxDefaultPosition, wxDefaultSize, 0 );
	m_installNow->SetMinSize( wxSize( 120,-1 ) );
	
	bSizer121->Add( m_installNow, 0, wxALL, 5 );
	
	
	bSizer121->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_dontInstall = new wxButton( m_installButtonsPanel, wxID_ANY, _("Don't install now"), wxDefaultPosition, wxDefaultSize, 0 );
	m_dontInstall->SetMinSize( wxSize( 120,-1 ) );
	
	bSizer121->Add( m_dontInstall, 0, wxALL, 5 );
	
	
	m_installButtonsPanel->SetSizer( bSizer121 );
	m_installButtonsPanel->Layout();
	bSizer121->Fit( m_installButtonsPanel );
	bSizer89->Add( m_installButtonsPanel, 0, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );
	
	
	this->SetSizer( bSizer89 );
	this->Layout();
	
	this->Centre( wxBOTH );
	
	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( FoundUpdatesDialogBase::OnClose ) );
	this->Connect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( FoundUpdatesDialogBase::OnInitDialog ) );
	m_products->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( FoundUpdatesDialogBase::productsOnListItemSelected ), NULL, this );
	this->Connect( m_moreInfo->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( FoundUpdatesDialogBase::moreInfoOnMenuSelection ) );
	this->Connect( m_moreInfo->GetId(), wxEVT_UPDATE_UI, wxUpdateUIEventHandler( FoundUpdatesDialogBase::moreInfoOnUpdateUI ) );
	this->Connect( m_manualDownload->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( FoundUpdatesDialogBase::manualDownloadOnMenuSelection ) );
	this->Connect( m_manualDownload->GetId(), wxEVT_UPDATE_UI, wxUpdateUIEventHandler( FoundUpdatesDialogBase::manualDownloadOnUpdateUI ) );
	m_updateNow->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( FoundUpdatesDialogBase::updateNowOnButtonClick ), NULL, this );
	m_skipUpdate->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( FoundUpdatesDialogBase::skipUpdateOnButtonClick ), NULL, this );
	m_dontUpdate->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( FoundUpdatesDialogBase::dontUpdateOnButtonClick ), NULL, this );
	m_installNow->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( FoundUpdatesDialogBase::installNowOnButtonClick ), NULL, this );
	m_dontInstall->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( FoundUpdatesDialogBase::dontInstallOnButtonClick ), NULL, this );
}

FoundUpdatesDialogBase::~FoundUpdatesDialogBase()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( FoundUpdatesDialogBase::OnClose ) );
	this->Disconnect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( FoundUpdatesDialogBase::OnInitDialog ) );
	m_products->Disconnect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( FoundUpdatesDialogBase::productsOnListItemSelected ), NULL, this );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( FoundUpdatesDialogBase::moreInfoOnMenuSelection ) );
	this->Disconnect( wxID_ANY, wxEVT_UPDATE_UI, wxUpdateUIEventHandler( FoundUpdatesDialogBase::moreInfoOnUpdateUI ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( FoundUpdatesDialogBase::manualDownloadOnMenuSelection ) );
	this->Disconnect( wxID_ANY, wxEVT_UPDATE_UI, wxUpdateUIEventHandler( FoundUpdatesDialogBase::manualDownloadOnUpdateUI ) );
	m_updateNow->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( FoundUpdatesDialogBase::updateNowOnButtonClick ), NULL, this );
	m_skipUpdate->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( FoundUpdatesDialogBase::skipUpdateOnButtonClick ), NULL, this );
	m_dontUpdate->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( FoundUpdatesDialogBase::dontUpdateOnButtonClick ), NULL, this );
	m_installNow->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( FoundUpdatesDialogBase::installNowOnButtonClick ), NULL, this );
	m_dontInstall->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( FoundUpdatesDialogBase::dontInstallOnButtonClick ), NULL, this );
	
	delete m_menu1; 
}
