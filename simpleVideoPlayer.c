#include <stdlib.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <vlc/vlc.h>

#define BORDER_WIDTH 6

void destroy(GtkWidget *widget, gpointer data);
void player_widget_on_realize(GtkWidget *widget, gpointer data);
void on_open(GtkWidget *widget, gpointer data);
void open_media(const char* uri);
void on_playpause(GtkWidget *widget, gpointer data);
void on_stop(GtkWidget *widget, gpointer data);
void play(void);
void pause_player(void);
gboolean _update_scale(gpointer data);
void on_value_change(GtkWidget *widget, gpointer data);

libvlc_media_t *media;
libvlc_media_player_t *media_player;
libvlc_instance_t *vlc_inst;

GtkWidget *playpause_button,
          *play_icon_image,
          *pause_icon_image,
          *stop_icon_image,
		  *process_scale;

GtkAdjustment *process_adjuest;

float video_length, current_play_time;

void destroy(GtkWidget *widget, gpointer data) {
    gtk_main_quit();
}

void player_widget_on_realize(GtkWidget *widget, gpointer data) {
    libvlc_media_player_set_xwindow ( (libvlc_media_player_t*)data, 
                                      GDK_WINDOW_XID(gtk_widget_get_window(widget)));
}


void on_open(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;

    dialog = gtk_file_chooser_dialog_new ("open file", GTK_WINDOW(widget), 
                                          action, 
                                          _("Cancel"), GTK_RESPONSE_CANCEL, 
                                          _("Open"), GTK_RESPONSE_ACCEPT, 
                                          NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *uri;
        uri = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(dialog));
        open_media(uri);
        g_free(uri);
    }
    gtk_widget_destroy(dialog);
}

void open_media(const char* uri) {
    media = libvlc_media_new_location(vlc_inst, uri);
    libvlc_media_player_set_media(media_player, media);

	current_play_time = 0.0f;
	gtk_scale_set_value_pos (GTK_SCALE(process_scale), 
                             current_play_time/video_length*100);

    play();
	g_timeout_add(500, _update_scale, process_scale);
    libvlc_media_release(media);
}

void on_playpause(GtkWidget *widget, gpointer data) {
    if (libvlc_media_player_is_playing(media_player) == 1) {
        pause_player();
    }
    else {
        play();
    }
}

void on_stop(GtkWidget *widget, gpointer data) {
    pause_player();
    libvlc_media_player_stop(media_player);
}


void on_value_change(GtkWidget *widget, gpointer data) {
	float scale_value = gtk_adjustment_get_value(process_adjuest);
	//printf("%f\n",scale_value);
	libvlc_media_player_set_position(media_player, scale_value/100);
}

gboolean _update_scale(gpointer data){
	// 获取当前打开视频的长度，时间单位为ms
	video_length = libvlc_media_player_get_length(media_player);
	current_play_time = libvlc_media_player_get_time(media_player);

	g_signal_handlers_block_by_func(G_OBJECT(process_scale), on_value_change, NULL);
	gtk_adjustment_set_value(process_adjuest,current_play_time/video_length*100);
	g_signal_handlers_unblock_by_func(G_OBJECT(process_scale), on_value_change, NULL);
	return G_SOURCE_CONTINUE;
}

void play(void) {
    libvlc_media_player_play(media_player);
    pause_icon_image = gtk_image_new_from_icon_name("media-playback-pause", GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image(GTK_BUTTON(playpause_button), pause_icon_image);
}

void pause_player(void) {
    libvlc_media_player_pause(media_player);
    play_icon_image = gtk_image_new_from_icon_name("media-playback-start", GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image(GTK_BUTTON(playpause_button), play_icon_image);
}

int main( int argc, char *argv[] ) {
    GtkWidget *window,
              *vbox,
			  *hbox,
              *menubar,
              *filemenu,
              *fileitem,
              *filemenu_openitem,
              *player_widget,
              *hbuttonbox,
              *stop_button;

    gtk_init (&argc, &argv);

    // setup window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    g_signal_connect(window, "destroy", G_CALLBACK(destroy), NULL);
    gtk_container_set_border_width (GTK_CONTAINER (window), 0);
    gtk_window_set_title(GTK_WINDOW(window), "GTK+ libVLC Demo");

    //setup box
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, FALSE);
    gtk_container_add(GTK_CONTAINER(window), vbox);
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, FALSE);

    //setup menu
    menubar = gtk_menu_bar_new();
    filemenu = gtk_menu_new();
    fileitem = gtk_menu_item_new_with_label ("File");
    filemenu_openitem = gtk_menu_item_new_with_label("Open");
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), filemenu_openitem);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(fileitem), filemenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), fileitem);

    gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);
    g_signal_connect(filemenu_openitem, "activate", G_CALLBACK(on_open), window);

    //setup player widget
    player_widget = gtk_drawing_area_new();
    gtk_box_pack_start(GTK_BOX(vbox), player_widget, TRUE, TRUE, 0);

    //setup controls
    playpause_button = gtk_button_new_from_icon_name("media-playback-start", GTK_ICON_SIZE_BUTTON);
    stop_button = gtk_button_new_from_icon_name("media-playback-stop", GTK_ICON_SIZE_BUTTON);

    g_signal_connect(playpause_button, "clicked", G_CALLBACK(on_playpause), NULL);
    g_signal_connect(stop_button, "clicked", G_CALLBACK(on_stop), NULL);

    hbuttonbox = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_container_set_border_width(GTK_CONTAINER(hbuttonbox), BORDER_WIDTH);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(hbuttonbox), GTK_BUTTONBOX_START);

    gtk_box_pack_start(GTK_BOX(hbuttonbox), playpause_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbuttonbox), stop_button, FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(hbox), hbuttonbox, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);

    //setup vlc
    vlc_inst = libvlc_new(0, NULL);
    media_player = libvlc_media_player_new(vlc_inst);
    g_signal_connect(G_OBJECT(player_widget), "realize", G_CALLBACK(player_widget_on_realize), media_player);


	//setup scale
	process_adjuest = gtk_adjustment_new(0.00, 0.00, 100.00, 1.00, 0.00, 0.00);
	process_scale = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL,process_adjuest);
	gtk_box_pack_start(GTK_BOX(hbox), process_scale, TRUE, TRUE, 0);
	gtk_scale_set_draw_value (GTK_SCALE(process_scale), FALSE);
	gtk_scale_set_has_origin (GTK_SCALE(process_scale), TRUE);
	gtk_scale_set_value_pos(GTK_SCALE(process_scale), 5);
	g_signal_connect(G_OBJECT(process_scale),"value_changed", G_CALLBACK(on_value_change), NULL);

    gtk_widget_show_all(window);
    gtk_main ();

    libvlc_media_player_release(media_player);
    libvlc_release(vlc_inst);
    return 0;
}
