/*  
    NetSpeed (C) Raja Jamwal 2011, www.experiblog.co.cc

    Distributed under GNU GPL License

    NetSpeed, determines the realtime network speed through kernel network interfaces
    through ifconfig utility 
    Copyright (C) 2011  Raja Jamwal

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <cairo.h>
#include <gtk/gtk.h>
#include <cstring>
#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

#define GRAPH_POINTS		200
#define WIDTH_ERROR		40
#define GRAPH_TOP_PAD		30
#define NUMBER_OF_DIVISIONS 	10

// Some variables
// Our linked gui file

extern char _binary_gui_glade_start[];

// TODO : Make dynamic number of graphs
// Number of graphs to draw

int number_of_graphs = 1;

// Our custom widget

GtkWidget *drawing;

// Previous bytes
unsigned int recv[5];
// Speed of all network interfaces in KB/s
unsigned int speed[5];

// Execute command in terminal and read output
std::string exec(char* cmd) {
    FILE* pipe = popen(cmd, "r");
    if (pipe == NULL) return "ERROR";
    char buffer[128];
    std::string result = "";
    while(!feof(pipe)) {
        if(fgets(buffer, 128, pipe) != NULL)
                result += buffer;
    }
    pclose(pipe);
    return result;
}

// Struct to hold data about a graph
struct data{
int val[GRAPH_POINTS];
data()
{
	for(int i=0; i<=GRAPH_POINTS ; i++){
 		val[i] =0;
	}
}
double r,g,b; // Color info to draw lines and gradient
char label[10]; // Network speed in this case
string information; // Any information, here network interace info
};

// create data holder for our graphs, here 1
data * info_for_graph;


bool first = true; // FIXME: this var be useless

// This functions collects information every 1 sec

gboolean info_d (gpointer data) {

info_for_graph[0].information = exec("ifconfig | grep \"Link encap:\" | sed 's/Link encap://' && ifconfig | grep \"inet addr:\"");
// calculate speed 
char * pch;
char temp[300];
strcpy(temp, exec("ifconfig | grep -o \"RX bytes:[^ ]*\" | sed 's/RX bytes://'").c_str());
pch = strtok ((char *)temp,"\n");
int gar_inc =0;

while (pch != NULL){

    unsigned int retr = atoi(pch);
    speed[gar_inc]= (retr-(first!=true?recv[gar_inc]:retr))/1000;
    recv[gar_inc] =retr;
    pch = strtok (NULL, "\n");
    gar_inc++;
    first = false;
}
return TRUE;
}

// return speed of active network interface

unsigned int current_speed(){
  unsigned int s=0;
  for(int i=0; i<5; i++)
	if(s<speed[i]) s = speed[i];
  return s;
}

// Does almost everything, from managing data to re-drawing our custom widget

gboolean spd (gpointer data) {

for(int p = 0; p<number_of_graphs; p++){

int ji=current_speed();
  
for(int i=0; i<=GRAPH_POINTS; i++)  {

  sprintf(info_for_graph[p].label, "%i KB/s", ji);
  if(i==GRAPH_POINTS){

    info_for_graph[p].val[GRAPH_POINTS] = ji;
  } else{
  if(info_for_graph[p].val[i]==0){

	info_for_graph[p].val[i] = ji; 
	break;

	}else{

   	info_for_graph[p].val[i]= (info_for_graph[p].val[i+1]==0?info_for_graph[p].val[i]:info_for_graph[p].val[i+1]);
	continue;
  }
 }
}

}
// let's redraw our widget
gtk_widget_queue_draw(drawing);

return TRUE;
}


// draws our widget

static gboolean
on_expose_event(GtkWidget      *widget,
                  GdkEventExpose *event,
                  gpointer        data)
{
  cairo_t *cr;

  cr = gdk_cairo_create(widget->window);

  int width = widget->allocation.width, height = widget->allocation.height; 
  cairo_rectangle(cr, 0, 0, width, height);
  cairo_set_source_rgb(cr, 0.12, 0.12, 0.12);
  cairo_fill(cr);  
 
  cairo_set_source_rgb(cr, 1, 1, 1);
  cairo_set_line_width (cr, 0.1);

  // draw our grid
  // draw horizontal lines
  for(int i=0; i<height ; i+=10){
  cairo_move_to(cr, 0, i);
  cairo_line_to(cr, width, i);
  }
  cairo_stroke(cr);

  // draw vertical lines
  for(int i=0; i<width ; i+=10){
  cairo_move_to(cr, i, 0);
  cairo_line_to(cr, i, height);
  }

  cairo_stroke(cr);

  cairo_set_line_width (cr, 2);
  cairo_set_source_rgb(cr, 0, 0, 0);
  
  // Calculate maximum height among all graphs
  int max = 0;
  for(int p=0; p<number_of_graphs; p++){
  for(int k=0; k<=GRAPH_POINTS; k++){
    if (info_for_graph[p].val[k]>max){
 		max= info_for_graph[p].val[k];}    
  	}
  }
  
  // draw graph data
  for(int p=0; p<number_of_graphs; p++){

  // information
  cairo_set_source_rgba (cr, 1, 1, 1, 0.5);
  cairo_select_font_face (cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size (cr, 13);
  
  char * pch;
  // store a temp copy of graph info for strok to work
  char temp[300];
  strcpy(temp, info_for_graph[p].information.c_str());

  pch = strtok ((char *)temp,"\n");
  int gar_inc =1;
  while (pch != NULL)
  {
    cairo_move_to(cr, 20, ((GRAPH_TOP_PAD)/2) * gar_inc);
    cairo_show_text (cr, pch);
    pch = strtok (NULL, "\n");
    gar_inc++;
  }

  // FIXME : inc is'nt calculated correct, double isn't enough, float is too intensive on processor
  int inc = width/(GRAPH_POINTS-WIDTH_ERROR);
  double steps = 0;
  cairo_set_source_rgb(cr, info_for_graph[p].r, info_for_graph[p].g, info_for_graph[p].b);
 
  int last_point = 0;
 
  cairo_move_to(cr , 0, height);
  cairo_line_to(cr, 0, height);
  
  for(int k=0; k<=GRAPH_POINTS; k++){
    double percent_height = (info_for_graph[p].val[k] * 100)/(max==0?1:max);
    last_point=steps;
    cairo_line_to(cr, steps, height-(((percent_height*height)/100)-GRAPH_TOP_PAD));
    steps+=inc;
  }

  cairo_line_to(cr, last_point, height);
  cairo_close_path(cr);
  cairo_stroke_preserve(cr);

  // create a gradient
  cairo_pattern_t *linpat = cairo_pattern_create_linear (width/2, 0, width/2, height);
  cairo_pattern_add_color_stop_rgba (linpat, 0, info_for_graph[p].r, info_for_graph[p].g, info_for_graph[p].b, 0.05);
  cairo_pattern_add_color_stop_rgba (linpat, 1, 0, 0, 0, 1);
  cairo_set_source(cr, linpat);
  cairo_fill(cr);

  }
  
  // Draw our scale
  cairo_set_source_rgb (cr, 1, 1, 1);
  cairo_select_font_face (cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size (cr, 13);

  double num = max/NUMBER_OF_DIVISIONS;

  for(int i=0;i<=NUMBER_OF_DIVISIONS; i++)
  {
  double percent_height = ((i*num) * 100)/(max==0?1:max);
  int wherey = height-(((percent_height*height)/100)-GRAPH_TOP_PAD);
  cairo_move_to(cr, 0, wherey);
  cairo_show_text (cr, "- ");
  cairo_move_to(cr, 2, wherey);
  char sp[10];sprintf(sp, "%i", (int)(i*num));
  cairo_show_text (cr, sp);
  }

  // Draw our labels, here network speed
  cairo_set_source_rgb (cr, 1, 1, 1);
  cairo_select_font_face (cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size (cr, 15);
 
  for(int i=0;i<width; i+=(width/number_of_graphs))
  {
  cairo_move_to(cr, i+5, GRAPH_TOP_PAD/2);
  cairo_show_text (cr, info_for_graph[i].label );
  
  }
  cairo_destroy(cr);

  return FALSE;
}

void close_app(GtkWidget* widget,gpointer user_data) {

	gtk_main_quit();
}

int main (int argc, char **argv) {
 
 info_for_graph = new data[number_of_graphs];

 for(int i=0; i<number_of_graphs; i++){
 info_for_graph[i].r=0;info_for_graph[i].g=1;info_for_graph[i].b=1;
 }

 GtkBuilder *gtkBuilder;
 GtkWidget *mainwin;
 gtk_set_locale();
 gtk_init (&argc, &argv);
 gtkBuilder= gtk_builder_new();

 // add gui data from linked data
 gtk_builder_add_from_string(gtkBuilder,_binary_gui_glade_start, -1, NULL);
 gtk_builder_connect_signals ( gtkBuilder, NULL );
 mainwin= GTK_WIDGET(gtk_builder_get_object(gtkBuilder,"window"));
 drawing = GTK_WIDGET(gtk_builder_get_object(gtkBuilder,"graph"));
 g_signal_connect(drawing, "expose-event", G_CALLBACK (on_expose_event), NULL);
 g_signal_connect(G_OBJECT(mainwin), "destroy", G_CALLBACK(close_app), NULL);
 // timer of re-drawing and managing arrays
 gint m_timer = g_timeout_add(300, spd, NULL);
 // timer of receving new data
 gint info = g_timeout_add(1000, info_d, NULL);
 g_object_unref ( G_OBJECT(gtkBuilder) );
 
 gtk_widget_show_all ( mainwin );
 gtk_main ();

 return 0;
}
