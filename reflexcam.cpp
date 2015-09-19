#include "reflexcam.h"
#include <fcntl.h>

ReflexCam::ReflexCam()
{
    gp_camera_new (&cam);
    context = gp_context_new();
    //gp_context_set_error_func(context, error_func, NULL);
    //gp_context_set_message_func(context, message_func, NULL);
    int ret = gp_camera_init(cam, context);
    if (ret < GP_OK) {
        printf("No camera auto detected.\n");
        gp_camera_free(cam);
        cam = NULL;
    }
    else
    {
        accessable=true;
    }
}

ReflexCam::~ReflexCam()
{
    if (cam) gp_camera_unref(cam);
    gp_context_unref(context);
}

bool ReflexCam::getImage(cv::Mat &mat)
{
    const char* filename = "temp";
    CameraFile *file;
    CameraFilePath camera_file_path;
    gp_camera_capture(cam, GP_CAPTURE_IMAGE, &camera_file_path, context);
    int fd = open(filename, O_CREAT | O_WRONLY, 0644);
    gp_file_new_from_fd(&file, fd);
    gp_camera_file_get(cam, camera_file_path.folder, camera_file_path.name, GP_FILE_TYPE_NORMAL, file, context);
    gp_camera_file_delete(cam, camera_file_path.folder, camera_file_path.name, context);
    mat = cv::imread(filename);

    return true;
}

bool ReflexCam::setFocus(int)
{
   return false;
}

void ReflexCam::error_func (GPContext* , const char *format, va_list args, void*) {
 fprintf  (stderr, "*** Contexterror ***\n");
 vfprintf (stderr, format, args);
 fprintf  (stderr, "\n");
}

void ReflexCam::message_func (GPContext *, const char *format, va_list args, void *) {
 vprintf (format, args);
 printf ("\n");
}

