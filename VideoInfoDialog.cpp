/*
 * @file VideoInfoDialog.cpp
 *
 * Implementation of a WX GUI window for video properties, etc.
 *
 * Created on: Sep 23, 2010
 * @author: Andrew Ford
 */

#include "VideoInfoDialog.h"
#include "VideoSource.h"
#include "Group.h"

#include "wx/stattext.h"

VideoInfoDialog::VideoInfoDialog( wxWindow* parent, RectangleBase* o )
    : wxDialog( parent, wxID_ANY, _("Video Info") ), obj( o )
{
    SetSize( wxSize( 200, 200 ) );
    wxStaticText* wxText = new wxStaticText( this, wxID_ANY, _("") );
    std::string text;
    text += "Name:\t" + obj->getName() + "\n";
    VideoSource* video = dynamic_cast<VideoSource*>( obj );
    if ( video )
    {
        text += "SDES name:\t" +
              video->getMetadata( VPMSession::VPMSESSION_SDES_NAME ) +
              "\n";
        text += "SDES cname:\t" +
              video->getMetadata( VPMSession::VPMSESSION_SDES_CNAME ) +
              "\n";
        text += "Location:\t" +
              video->getMetadata( VPMSession::VPMSESSION_SDES_LOC ) +
              "\n";
        text += "Codec:\t" +
              std::string( video->getPayloadDesc() ) +
              "\n";
    }
    text += "Grouped?\t" +
            std::string( obj->isGrouped() ? "Yes" : "No" ) +
            "\n";
    if ( obj->isGrouped() )
    {
        text += "Group:?\t" +
                obj->getGroup()->getName() +
                "\n";
    }
    wxText->SetLabel( wxString( text.c_str(), wxConvUTF8 ) );
}
