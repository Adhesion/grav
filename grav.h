#ifndef GRAV_H_
#define GRAV_H_

/**
 * @file grav.h
 * Header file for main grav app - contains the definition for the main class,
 * which acts as the main WX window/controller and OnInit() which acts as the
 * main().
 * @author Andrew Ford
 */

#include <wx/wx.h>

class GLCanvas;
class gravManager;
class TreeControl;

class gravApp : public wxApp
{
    
private:

    virtual bool OnInit();
    
    wxFrame* mainFrame;
    wxFrame* treeFrame;
    
    GLCanvas* canvas;
    TreeControl* tree;
    
    gravManager* grav;
    
};

#endif /*GRAV_H_*/
