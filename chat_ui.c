#include <gtk/gtk.h>
#include <string.h>
#include "com_with_server.h"
#include "error_dialog_ui.h"
#include "chat_ui.h"

GtkBuilder *builder;
GtkWidget *window;
gchar username[20];
gchar chat_friend_name[20];

void update_local_ui(gchar *name, gchar *msg){
    GtkWidget * msg_textview = GTK_WIDGET(gtk_builder_get_object(builder, "msg_textview"));
    GtkTextBuffer *msg_textbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW (msg_textview));

    GtkTextIter end;
    gtk_text_buffer_get_end_iter(msg_textbuffer, &end);

    gtk_text_buffer_insert (msg_textbuffer, &end, name, -1);
    gtk_text_buffer_insert (msg_textbuffer, &end, ":\n", -1);
    gtk_text_buffer_insert (msg_textbuffer, &end, msg, -1);
    gtk_text_buffer_insert (msg_textbuffer, &end, "\n\n", -1);
}

void receive_msg_from_server(gchar *friend_name, gchar * msg){
    update_local_ui(friend_name, msg);
}

gchar *get_type_textview_msg(){
    GtkWidget * type_textview = GTK_WIDGET (gtk_builder_get_object(builder, "type_textview"));
    GtkTextBuffer * type_textbuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (type_textview));
    
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter (type_textbuffer, &start);
    gtk_text_buffer_get_end_iter (type_textbuffer, &end);

    gchar * msg = gtk_text_buffer_get_text(type_textbuffer, &start, &end, TRUE);
    gtk_text_buffer_set_text (type_textbuffer, "", -1);
    return msg;
}

void send_button_press(GtkWidget *widget, gpointer *data){
    // 1. Get chat_friend_name and the message
    if (strcmp(chat_friend_name, "") == 0){
        return;
    }
    gchar *msg = get_type_textview_msg();
    if (strlen(msg) <= 0){
        return;
    }

    // 2. Send to server
    gboolean send_msg_to_server_succeed = send_msg_to_server(chat_friend_name, msg);
    if (send_msg_to_server_succeed == FALSE){
        error_dialog (window, "Connet faild!");
        return;
    }

    // 3. Set the text buffer in UI
    update_local_ui(chat_friend_name, msg);
}

void uTalk_friend_selected (GtkWidget *widget, GtkWidget *name_label){
    // Update chatwith label
    GtkWidget *chatwith_label = GTK_WIDGET (gtk_builder_get_object(builder, "chatwith_label"));
    const gchar *name = gtk_label_get_text ((GtkLabel *)widget);
    strcpy(chat_friend_name, name);
    gtk_label_set_text ((GtkLabel *)chatwith_label, chat_friend_name);

    // Clear msg_textview
    GtkWidget *msg_textview = GTK_WIDGET (gtk_builder_get_object(builder, "msg_textview"));
    GtkTextBuffer *msg_textbuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (msg_textview));
    gtk_text_buffer_set_text (msg_textbuffer, "", -1);

    // Send chat_friend_name to server
    gboolean build_chat_with_server_succeed = build_chat_with_server(chat_friend_name);
    if (build_chat_with_server_succeed == FALSE){
        error_dialog (window, "Connet faild!");
        return;
    }
}

GtkWidget *uTalk_friend_new (gchar *avator_path, gchar *friend_name){
    GtkWidget *avator_image = gtk_image_new_from_file (avator_path);
    
    GtkWidget *name_label = gtk_label_new (friend_name);
    PangoAttrList *name_attr_list = pango_attr_list_new ();
    PangoAttribute *name_attr_scale = pango_attr_scale_new (1.2);
    pango_attr_list_insert (name_attr_list, name_attr_scale);
    gtk_label_set_attributes ((GtkLabel *)name_label, name_attr_list);
    pango_attr_list_unref (name_attr_list);
    
    GtkWidget *msg_label = gtk_label_new ("");
    
    GtkWidget *name_and_msg_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_set_homogeneous (GTK_BOX(name_and_msg_box), TRUE);
    gtk_box_pack_start (GTK_BOX(name_and_msg_box), name_label, FALSE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX(name_and_msg_box), msg_label, FALSE, TRUE, 0);
    
    GtkWidget *friend_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (GTK_BOX(friend_box), avator_image, FALSE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX(friend_box), name_and_msg_box, FALSE, TRUE, 0);

    GtkWidget *friend_eventbox = gtk_event_box_new ();
    gtk_widget_set_events (friend_eventbox, GDK_BUTTON_PRESS_MASK);
    g_signal_connect_swapped (G_OBJECT(friend_eventbox), "button-press-event", G_CALLBACK(uTalk_friend_selected), name_label);
    gtk_container_add (GTK_CONTAINER(friend_eventbox), friend_box);

    return friend_eventbox;
}

void add_friend (GtkWidget *widget, gpointer *data){
    GtkWidget *friends_listbox = GTK_WIDGET(gtk_builder_get_object(builder, "friends_listbox"));
    GtkWidget *friend = uTalk_friend_new("dada.jpg", "DaDax");
    gtk_list_box_insert ((GtkListBox *)friends_listbox, friend, 0);
    gtk_widget_show_all (friends_listbox);
}

void load_friends_list(){
    gint friends_num;
    gchar *friends_list[20];
    request_friends_list_from_server(username, &friends_num, friends_list);
    GtkWidget *friends_listbox = GTK_WIDGET(gtk_builder_get_object(builder, "friends_listbox"));
    for (int i = 0; i < friends_num; i++){
        GtkWidget *friend = uTalk_friend_new("dada.jpg", friends_list[i]);
        gtk_list_box_insert ((GtkListBox *)friends_listbox, friend, -1);
    }
}

void tmp(){
    receive_msg_from_server("ABC", "Hello");
}

gboolean chat_ui (const gchar *rev_username){
    gtk_init (NULL, NULL);
    strcpy(username, rev_username);

    builder = gtk_builder_new_from_file ("chat_ui.glade");

    window = GTK_WIDGET (gtk_builder_get_object(builder, "window"));
    g_signal_connect (G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *send_button = GTK_WIDGET(gtk_builder_get_object(builder, "send_button"));
    g_signal_connect (G_OBJECT(send_button), "clicked", G_CALLBACK(send_button_press), NULL);

    GtkWidget *add_friend_button = GTK_WIDGET (gtk_builder_get_object(builder, "add_friend_button"));
    g_signal_connect (G_OBJECT(add_friend_button), "clicked", G_CALLBACK(add_friend), NULL);

    GtkWidget *tmp_button = GTK_WIDGET (gtk_builder_get_object(builder, "tmp_button"));
    g_signal_connect_swapped (G_OBJECT(tmp_button), "clicked", G_CALLBACK(tmp), NULL);

    memset(chat_friend_name, 0, sizeof(chat_friend_name));

    load_friends_list();

    gtk_widget_show_all(window);
    gtk_main();
    return TRUE;
}
