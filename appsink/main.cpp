#include <gst/gst.h>
#include <string.h>
#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>

typedef struct _CustomData
{

	GMainLoop *loop;
	
//src
	GstBus *src_bus; GstElement *src_pipeline;
//common
	GstElement *app_sink, *video_source;


} CustomData;

/* The appsink has received a buffer */
static GstFlowReturn new_sample(GstElement *sink, CustomData *data)
{
    GstSample *sample; 
    GstFlowReturn ret = GST_FLOW_OK;

  /* Retrieve the buffer */
    g_signal_emit_by_name (data->app_sink, "pull-sample", &sample, NULL);

    if (sample)
    {        
        /* The only thing we do in this example is print a * to indicate a received buffer */
	g_print("*\n");
  	 
        gst_sample_unref (sample);

        return GST_FLOW_OK;

    }
    else
    {
        g_print ("could not make snapshot\n");
        
        return GST_FLOW_ERROR;
    }

    return GST_FLOW_ERROR;
}

/* called when we get a GstMessage from the source pipeline when we get EOS, we
 * notify the appsrc of it. */
static void src_message_cb(GstBus *bus, GstMessage *msg, CustomData *data)
{
    switch (GST_MESSAGE_TYPE (msg)) {
    case GST_MESSAGE_ERROR: {
        GError *err;
        gchar *debug_info;

        gst_message_parse_error(msg, &err, &debug_info);
        g_print("Error: %s\n", err->message);
        g_error_free(err);
        g_free(debug_info);

        gst_element_set_state(data->src_pipeline, GST_STATE_READY);
        g_main_loop_quit(data->loop);
        break;
    }
    case GST_MESSAGE_EOS: {
        /* end-of-stream */
        gst_element_set_state(data->src_pipeline, GST_STATE_READY);
        g_main_loop_quit(data->loop);
        break;
    }
    default:
        /* Unhandled message */
        break;
    }
}

int
main (int argc, char *argv[])
{
    CustomData data;
    
  /* Initialize GStreamer */
    gst_init(NULL, NULL);

  /* Create a GLib Main Loop and set it to run */
    data.loop = g_main_loop_new(NULL, FALSE);
        
    /* Create the elements */	
    data.video_source 		= gst_element_factory_make("videotestsrc", "video_source");
    data.app_sink 		= gst_element_factory_make("appsink", "app_sink");
    
    /* Create the empty pipeline */
    data.src_pipeline 	= gst_pipeline_new("src_pipeline");
    
    if(!data.src_pipeline || !data.video_source || !data.app_sink) {
        g_printerr ("Not all elements could be created.\n");
        return -1;
    }

    g_object_set(data.app_sink, "emit-signals", TRUE, NULL);
    g_signal_connect(data.app_sink, "new-sample", G_CALLBACK(new_sample), &data);
    
    /* Build the pipeline */
    gst_bin_add_many(GST_BIN(data.src_pipeline), data.video_source, data.app_sink, NULL);
    
    /* Link the elements together */
    if(gst_element_link(data.video_source, data.app_sink) != TRUE)
    {
        g_printerr("Elements could not be linked.\n");
        gst_object_unref(data.src_pipeline);
        return -1;
    }

    /* Add function to watch bus */
    data.src_bus = gst_element_get_bus(data.src_pipeline);
    gst_bus_add_signal_watch(data.src_bus);
    g_signal_connect(G_OBJECT(data.src_bus), "message", (GCallback)src_message_cb, &data);
    gst_object_unref(data.src_bus);
            
    gst_element_set_state(data.src_pipeline, GST_STATE_PLAYING);
    
    g_main_loop_run(data.loop);
    
    gst_element_set_state(data.src_pipeline, GST_STATE_NULL);
    
    /* Free the buffer now that we are done with it */
    gst_object_unref(data.src_pipeline);

    g_main_loop_unref (data.loop);
        
    return 0;
}
