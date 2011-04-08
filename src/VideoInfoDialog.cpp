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

#include "wx/stattext.h"

VideoInfoDialog::VideoInfoDialog( wxWindow* parent, RectangleBase* o )
    : wxDialog( parent, wxID_ANY, _("Video Info") ), obj( o )
{
    SetSize( wxSize( 250, 150 ) );
    wxStaticText* wxText = new wxStaticText( this, wxID_ANY, _("") );
    std::string text;
    text += "Name:\t" + obj->getName() + "\n";
    VideoSource* video = dynamic_cast<VideoSource*>( obj );
    if ( video )
    {
        text += "RTP name:\t" +
              video->getMetadata( VPMSession::VPMSESSION_SDES_NAME ) +
              "\n";
        text += "RTP cname:\t" +
              video->getMetadata( VPMSession::VPMSESSION_SDES_CNAME ) +
              "\n";
        text += "Location:\t" +
              video->getMetadata( VPMSession::VPMSESSION_SDES_LOC ) +
              "\n";
        text += "Codec:\t" +
              std::string( video->getPayloadDesc() ) +
              "\n";
        char width[10];
        char height[10];
        sprintf( width, "%u", video->getVideoWidth() );
        sprintf( height, "%u", video->getVideoHeight() );
        text += "Resolution:\t" +
              std::string( width ) + " x " + std::string( height ) +
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
