#include <gst/gst.h>
#include <string.h>
#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>

typedef struct _CustomData
{

	GMainLoop *loop;
	
//src
	GstBus *src_bus; GstElement *src_pipeline; GstCaps *src_caps;

//sink
	GstBus *sink_bus; GstElement *sink_pipeline; GstCaps *sink_caps;

//common
	GstElement *app_sink, *app_source, *video_sink, *video_convert, *video_source, *video_caps1;


} CustomData;

/* The appsink has received a buffer */
static GstFlowReturn new_sample(GstElement *sink, CustomData *data)
{
    GstSample *sample; 
    GstBuffer *buffer;
    GstFlowReturn ret = GST_FLOW_OK;

    g_signal_emit_by_name (data->app_sink, "pull-sample", &sample, NULL);

    if (sample)
    {        
        buffer = gst_sample_get_buffer (sample);

  	g_signal_emit_by_name (data->app_source, "push-buffer", buffer, &ret);
  	 
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


static void sink_message_cb(GstBus *bus, GstMessage *msg, CustomData *data)
{
    switch (GST_MESSAGE_TYPE (msg)) {
    case GST_MESSAGE_ERROR: {
        GError *err;
        gchar *debug_info;

        gst_message_parse_error(msg, &err, &debug_info);
        g_print("Error: %s\n", err->message);
        g_error_free(err);
        g_free(debug_info);

        gst_element_set_state(data->sink_pipeline, GST_STATE_READY);
        g_main_loop_quit(data->loop);
        break;
    }
    case GST_MESSAGE_EOS: {
        /* end-of-stream */
        gst_element_set_state(data->sink_pipeline, GST_STATE_READY);
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

    data.loop = g_main_loop_new(NULL, FALSE);
        
    /* Create the elements */	
    data.video_source 		= gst_element_factory_make("videotestsrc", "video_source");
    data.video_caps1		= gst_element_factory_make ("capsfilter","video_caps");  
    data.video_convert 		= gst_element_factory_make("videoconvert", "video_convert");
    data.app_sink 		= gst_element_factory_make("appsink", "app_sink");
    
    /* Create the empty pipeline */
    data.src_pipeline 	= gst_pipeline_new("src_pipeline");
    
    if(!data.src_pipeline || !data.video_source || !data.video_convert || !data.app_sink) {
        g_printerr ("Not all elements could be created.\n");
        return -1;
    }
    
    /* Modify the caps properties */
    data.src_caps = gst_caps_new_simple("video/x-raw",\
        "format", G_TYPE_STRING, "UYVY",\
        "width", G_TYPE_INT, 640, \
        "height", G_TYPE_INT, 480, \
        "framerate", GST_TYPE_FRACTION, 30, 1,
        NULL);
     
    g_object_set (G_OBJECT (data.video_caps1), "caps", data.src_caps, NULL);
    
    g_object_set(data.app_sink, "emit-signals", TRUE, NULL);
    g_signal_connect(data.app_sink, "new-sample", G_CALLBACK(new_sample), &data);
    gst_caps_unref(data.src_caps);
    
    /* Build the pipeline */
    gst_bin_add_many(GST_BIN(data.src_pipeline), data.video_source, data.video_caps1, data.video_convert, data.app_sink, NULL);
    
    /* Link the elements together */
    if(gst_element_link_many(data.video_source, data.video_caps1, data.video_convert, NULL) != TRUE || \
        gst_element_link(data.video_convert, data.app_sink) != TRUE)
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

////////////////////////////////////////////////////////////////////////////////////////////////////

    data.app_source 		= gst_element_factory_make("appsrc", "app_source");
    data.video_convert		= gst_element_factory_make("videoconvert", "video_convert");
    data.video_sink 		= gst_element_factory_make("xvimagesink", "video_sink");

    /* Create the empty pipeline */
    data.sink_pipeline 	= gst_pipeline_new("sink_pipeline");
    
    if(!data.sink_pipeline || !data.app_source || !data.video_convert ||!data.video_sink)
    {
        g_printerr ("Not all elements could be created.\n");
        return -1;
    }

    /* Modify the source properties */
    g_object_set (data.app_source, "stream-type", 0, "is-live", 1, "do-timestamp", 0, "format", GST_FORMAT_TIME, NULL);
    
    /* Modify the caps properties */
    data.sink_caps = gst_caps_new_simple("video/x-raw", \
        "format", G_TYPE_STRING, "UYVY", \
        "width", G_TYPE_INT, 640, \
        "height", G_TYPE_INT, 480, \
        "framerate", GST_TYPE_FRACTION, 30, 1, \
        NULL);
	
    g_object_set(data.app_source, "caps", data.sink_caps, NULL);
    gst_caps_unref(data.sink_caps);
    
    g_object_set(data.video_sink, "sync", FALSE, "async", FALSE, NULL);

    /* Build the pipeline */
    gst_bin_add_many(GST_BIN(data.sink_pipeline), data.app_source, data.video_convert, data.video_sink, NULL);
    
    /* Link the elements together */
    if(gst_element_link_many(data.app_source, data.video_convert, data.video_sink, NULL) != TRUE) {
        g_printerr("Elements could not be linked.\n");
        gst_object_unref(data.sink_pipeline);
        gst_caps_unref(data.sink_caps);
        return -1;
    }
    
    /* Add function to watch bus */
    data.sink_bus = gst_element_get_bus(data.sink_pipeline);
    gst_bus_add_signal_watch(data.sink_bus);
    g_signal_connect(G_OBJECT(data.sink_bus), "message", (GCallback)sink_message_cb, &data);
    gst_object_unref (data.sink_bus);
            
    gst_element_set_state(data.src_pipeline, GST_STATE_PLAYING);
    gst_element_set_state(data.sink_pipeline, GST_STATE_PLAYING);
    
    g_main_loop_run(data.loop);
    
    gst_element_set_state(data.src_pipeline, GST_STATE_NULL);
    gst_element_set_state(data.sink_pipeline, GST_STATE_NULL);
    
    gst_object_unref(data.src_pipeline);
    gst_object_unref(data.sink_pipeline);
    g_main_loop_unref (data.loop);
        
    return 0;
}
