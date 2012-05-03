/*
 * @file VideoInfoDialog.cpp
 *
 * Implementation of a WX GUI window for video properties, etc.
 *
 * Created on: Sep 23, 2010
 * @author: Andrew Ford
 * Copyright (C) 2011 Rochester Institute of Technology
 *
 * This file is part of grav.
 *
 * grav is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * grav is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with grav.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "VideoInfoDialog.h"
#include "VideoSource.h"
#include "Group.h"

#include <wx/stattext.h>
#include <wx/sizer.h>

VideoInfoDialog::VideoInfoDialog( wxWindow* parent, RectangleBase* o )
    : wxDialog( parent, wxID_ANY, _("Video Info") ), obj( o )
{
    SetSize( wxSize( 250, 150 ) );
    wxStaticText* labelText = new wxStaticText( this, wxID_ANY, _("") );
    wxStaticText* infoText = new wxStaticText( this, wxID_ANY, _("") );
    std::string labelTextStd, infoTextStd;

    labelTextStd += "Name:\n";
    infoTextStd += obj->getName() + "\n";
    VideoSource* video = dynamic_cast<VideoSource*>( obj );
    if ( video )
    {
        labelTextStd += "RTP name:\n";
        infoTextStd += video->getMetadata( VPMSession::VPMSESSION_SDES_NAME ) +
                "\n";
        labelTextStd += "RTP cname:\n";
        infoTextStd += video->getMetadata( VPMSession::VPMSESSION_SDES_CNAME ) +
                "\n";
        labelTextStd += "Location:\n";
        infoTextStd += video->getMetadata( VPMSession::VPMSESSION_SDES_LOC ) +
                "\n";
        labelTextStd += "Codec:\n";
        infoTextStd += std::string( video->getPayloadDesc() ) + "\n";
        labelTextStd += "Alternate Address:\n";
        infoTextStd += video->getAltAddress() + "\n";
        char width[10];
        char height[10];
        sprintf( width, "%u", video->getVideoWidth() );
        sprintf( height, "%u", video->getVideoHeight() );
        labelTextStd += "Resolution:\n";
        infoTextStd += std::string( width ) + " x " + std::string( height ) +
                "\n";
    }
    labelTextStd += "Grouped?";
    infoTextStd += std::string( obj->isGrouped() ? "Yes" : "No" );
    if ( obj->isGrouped() )
    {
        labelTextStd += "\nGroup:";
        infoTextStd += "\n" + obj->getGroup()->getName();
    }

    labelText->SetLabel( wxString( labelTextStd.c_str(), wxConvUTF8 ) );
    infoText->SetLabel( wxString( infoTextStd.c_str(), wxConvUTF8 ) );

    wxBoxSizer* textSizer = new wxBoxSizer( wxHORIZONTAL );
    textSizer->Add( labelText, wxSizerFlags(0).Align(0).Border( wxALL, 10 ) );
    textSizer->Add( infoText, wxSizerFlags(0).Align(0).Border( wxALL, 10 ) );

    SetSizer( textSizer );
    textSizer->SetSizeHints( this );
}
