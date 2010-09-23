/*
 * @file VideoInfoDialog.h
 *
 * Definition of a WX GUI window for video properties, etc.
 *
 * Created on: Sep 23, 2010
 * @author: Andrew Ford
 */

#ifndef VIDEOINFODIALOG_H_
#define VIDEOINFODIALOG_H_

#include "wx/dialog.h"

class RectangleBase;

class VideoInfoDialog : public wxDialog
{

public:
    VideoInfoDialog( wxWindow* parent, RectangleBase* o );

private:
    RectangleBase* obj;

};

#endif /* VIDEOINFODIALOG_H_ */
