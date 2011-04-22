/*
 * @file VideoInfoDialog.h
 *
 * Definition of a WX GUI window for video properties, etc.
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

#ifndef VIDEOINFODIALOG_H_
#define VIDEOINFODIALOG_H_

#include <wx/dialog.h>

class RectangleBase;

class VideoInfoDialog : public wxDialog
{

public:
    VideoInfoDialog( wxWindow* parent, RectangleBase* o );

private:
    RectangleBase* obj;

};

#endif /* VIDEOINFODIALOG_H_ */
