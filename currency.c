/**
 *
 * Copyright (c) 2014 Shambler GMX, see the file AUTHORS for details.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <glib/gi18n.h>

#include <string.h>

#include <lxpanel/plugin.h>

#include <curl/curl.h>

typedef struct {
	unsigned int timer;
	Plugin * plugin;
	GtkWidget *main;
	GtkWidget *widget;
} Currency;

typedef struct __writer_buffer__
{
	char *data;
	unsigned int size;
	gboolean result;
} writer_buffer_t;

static int curl_writer(char *data, size_t size, size_t nmemb, writer_buffer_t *buffer)
{
	char *start, *end;

	start = strstr(data, "<span class=bld>");
	if(start != NULL)
	{
		start += 16;
		end = strstr(start, " ");
		if(end != NULL)
		{
			if(buffer->size > (end - start))
			{
				strncpy(buffer->data, start, end - start);
				buffer->data[end - start] = '\0';
				buffer->result = TRUE;
			}
		}
	}

	return size * nmemb;
}

gboolean get_currency(char *result, const unsigned int result_size, const char *amount, const char *from, const char *to)
{
	char url[256], buffer[256];
	gboolean ret = FALSE;
	CURLcode res;
	CURL *curl;
	writer_buffer_t writer_buffer;

	if(strlen(amount) == 0 || strlen(from) == 0 || strlen(to) == 0)
	{
		return FALSE;
	}

	snprintf(&url[0], 255, "http://www.google.com/finance/converter?a=%s&from=%s&to=%s", amount, from, to);

	writer_buffer.data = &buffer[0];
	writer_buffer.size = 256;
	writer_buffer.result = FALSE;

	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (X11; Linux x86_64; rv:31.0) Gecko/20100101 Firefox/31.0 Iceweasel/31.2.0");
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 600L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &curl_writer);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &writer_buffer);
	curl_easy_setopt(curl, CURLOPT_URL, url);

	res = curl_easy_perform(curl);
	if(res == CURLE_OK && writer_buffer.result)
	{
		if(strlen(buffer) > 2)
		{
			buffer[strlen(buffer) - 2] = '\0';
		}
		snprintf(result, result_size, "%s%s", buffer, to);
		ret = TRUE;
	}

	curl_easy_cleanup(curl);

	return ret;
}

gboolean button_press_event( GtkWidget *widget, GdkEventButton* event, Plugin* plugin)
{
	ENTER2;

	if( event->button == 1 ) /* left button */
	{
	}
	else if( event->button == 2 ) /* middle button */
	{
	}
	else if( event->button == 3 )  /* right button */
	{
	}

	RET2(TRUE);
}

static void *curl_thread(void *data)
{
	Currency *egz = (Currency *)data;
	char result[256];

	if(get_currency(result, 256, "1", "USD", "RUB"))
	{
		gtk_label_set_markup(GTK_LABEL(egz->widget), result);
	}
	else
	{
		gtk_label_set_markup(GTK_LABEL(egz->widget), "--");
	}

	return NULL;
}

static gint timer_event(Currency *egz)
{
	GThread *thread = g_thread_new("curl thread", &curl_thread, (gpointer)egz);

	return TRUE;
}

static int currency_constructor(Plugin *p, char** fp)
{
	Currency *egz;
	GThread *thread;

	ENTER;
	/* initialization */
	curl_global_init(CURL_GLOBAL_ALL);

	egz = g_new0(Currency, 1);
	egz->plugin = p;
	p->priv = egz;

	p->pwid = gtk_event_box_new();
	GTK_WIDGET_SET_FLAGS( p->pwid, GTK_NO_WINDOW );
	gtk_container_set_border_width( GTK_CONTAINER(p->pwid), 2 );

	egz->widget = gtk_label_new(" ");
	gtk_label_set_markup (GTK_LABEL(egz->widget), "--") ;

	gtk_container_add(GTK_CONTAINER(p->pwid), egz->widget);

	egz->main = p->pwid;

	g_signal_connect (G_OBJECT (p->pwid), "button_press_event", G_CALLBACK (button_press_event), (gpointer) p);

	egz->timer = g_timeout_add(3600 * 1000, (GSourceFunc) timer_event, (gpointer)egz); /* set timer */
	thread = g_thread_new("curl thread", &curl_thread, (gpointer)egz);

	gtk_widget_show(egz->widget); /* show plugin on panel */
	RET(TRUE);
}

static void applyConfig(Plugin* p)
{
	ENTER;
	RET();
}

static void config(Plugin *p, GtkWindow* parent) {
	ENTER;
	RET();
}

static void currency_destructor(Plugin *p)
{
	ENTER;
	curl_global_cleanup();
	Currency *egz = (Currency *)p->priv;
	g_source_remove(egz->timer);
	g_free(egz);
	RET();
}

static void save_config( Plugin* p, FILE* fp )
{
	ENTER;
	RET();
}

PluginClass currency_plugin_class = {

	PLUGINCLASS_VERSIONING,

	type : "currency",
	name : N_("Currency"),
	version: "0.6",
	description : N_("A simple plugin that shows the current rate of exchange selected."),

	constructor	: currency_constructor,
	destructor	: currency_destructor,
	config		: config,
	save		: save_config,
	panel_configuration_changed : NULL
};
