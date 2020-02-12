# pragma once

# include <sigogl/ui_button.h>
# include <sigogl/ui_check_button.h>
# include <sigogl/ws_viewer.h>

// Viewer for this example:
class MyViewer : public WsViewer
{  protected :
	enum MenuEv { EvAdd, EvMultiColor, EvInfo, EvExit };
	UiCheckButton* mcolorbut;
	GsVec lightPos = GsVec(0, 2, 5);

   public :
	MyViewer ( int x, int y, int w, int h, const char* l );
	void add_ui ();
	void add_mynode ( int n );
	virtual int handle_keyboard ( const GsEvent &e ) override;
	virtual int uievent ( int e ) override;
	void build_scene();
	void add_model(SnGroup* parentg, SnShape* s, GsVec p);
	void trace(int width, int height, int bounces);
	void render_pixel(GsVec evalPixel, int bounces);

};
