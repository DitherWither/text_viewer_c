/* text_viewer_c-window.c
 *
 * Copyright 2023 Vardhan Patil
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "config.h"

#include "text_viewer_c-window.h"

struct _TextViewerCWindow {
    AdwApplicationWindow parent_instance;

    /* Template widgets */
    GtkHeaderBar *header_bar;
    GtkTextView *main_text_view;
    GtkButton *open_button;
};

static void text_viewer_c_window__open_file_dialog(GAction *action,
                                                   GVariant *param,
                                                   TextViewerCWindow *self);

static void on_open_response(GtkNativeDialog *native, int response,
                             TextViewerCWindow *self);

static void open_file(TextViewerCWindow *self, GFile *file);

static void open_file_completed(GObject *source_object, GAsyncResult *res,
                                TextViewerCWindow *self);

G_DEFINE_FINAL_TYPE(TextViewerCWindow, text_viewer_c_window,
                    ADW_TYPE_APPLICATION_WINDOW)

static void text_viewer_c_window_class_init(TextViewerCWindowClass *klass) {
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    gtk_widget_class_set_template_from_resource(
        widget_class, "/com/example/TextViewerC/text_viewer_c-window.ui");

    gtk_widget_class_bind_template_child(widget_class, TextViewerCWindow,
                                         header_bar);

    gtk_widget_class_bind_template_child(widget_class, TextViewerCWindow,
                                         main_text_view);

    gtk_widget_class_bind_template_child(widget_class, TextViewerCWindow,
                                         open_button);
}

static void text_viewer_c_window_init(TextViewerCWindow *self) {
    g_autoptr(GSimpleAction) open_action;

    gtk_widget_init_template(GTK_WIDGET(self));

    open_action = g_simple_action_new("open", NULL);

    g_signal_connect(open_action, "activate",
                     G_CALLBACK(text_viewer_c_window__open_file_dialog), self);

    g_action_map_add_action(G_ACTION_MAP(self), G_ACTION(open_action));
}

void text_viewer_c_window__open_file_dialog(GAction *action G_GNUC_UNUSED,
                                            GVariant *param G_GNUC_UNUSED,
                                            TextViewerCWindow *self) {

    // Create a new file chooser dialog using the open mode
    GtkFileChooserNative *native = gtk_file_chooser_native_new(
        "Open File", GTK_WINDOW(self), GTK_FILE_CHOOSER_ACTION_OPEN, "_Open",
        "_Cancel");

    // Connect the response signal to the callback function
    g_signal_connect(native, "response", G_CALLBACK(on_open_response), self);
}

static void on_open_response(GtkNativeDialog *native, int response,
                             TextViewerCWindow *self) {
    // If the user selected a file...
    if (response == GTK_RESPONSE_ACCEPT) {
        // Get/cast the file chooser
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(native);

        // Get the location of the selected file
        g_autoptr(GFile) file = gtk_file_chooser_get_file(chooser);

        // and open it
        open_file(self, file);
    }
    // Release the file chooser
    g_object_unref(native);
}

static void open_file(TextViewerCWindow *self, GFile *file) {
    g_file_load_bytes_async(file, NULL,
                            (GAsyncReadyCallback)open_file_completed, self);
}

static void open_file_completed(GObject *source_object, GAsyncResult *res,
                                TextViewerCWindow *self) {
    GFile *file = G_FILE(source_object);

    g_autofree char *contents = NULL;
    gsize length = 0;

    g_autoptr(GError) error = NULL;

    // Complete the asynchronous file loading
    // and get the contents of the file or an error
    g_file_load_contents_finish(file, res, &contents, &length, NULL, &error);

    // Check for errors
    if (error != NULL) {
        g_printerr("Unable to open %s: %s", g_file_peek_path(file),
                   error->message);
        return;
    }

    // Make sure that the file is valid UTF-8
    if (!g_utf8_validate(contents, length, NULL)) {
        g_printerr("Unable to open %s:"
                   "The file is not valid UTF-8",
                   g_file_peek_path(file));
        return;
    }

    // Retrieve the text buffer from the text view
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(self->main_text_view);

    // Set the text buffer's text to the file's contents
    gtk_text_buffer_set_text(buffer, contents, length);

    // Reposition the cursor at the beginning of the buffer
    GtkTextIter start;
    gtk_text_buffer_get_start_iter(buffer, &start);
    gtk_text_buffer_place_cursor(buffer, &start);
}