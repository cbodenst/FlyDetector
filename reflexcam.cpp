#include <fcntl.h>
#include <iostream>

#include "reflexcam.h"

ReflexCam::ReflexCam() : cam(nullptr), context(nullptr)
{
    gp_camera_new(&cam);
    context   = gp_context_new();
    int error = gp_camera_init(cam, context);

    if (error < GP_OK) {
        std::cerr << "Could not auto-detect camera" << std::endl;
        gp_camera_free(cam);
        cam = nullptr;
        this->accessable = false;
    }
    else
    {
        this->accessable = true;
    }
}

bool ReflexCam::getImage(cv::Mat &mat)
{
    const char* filename = "temp";
    CameraFile*file;
    CameraFilePath camera_file_path;
    
    gp_camera_capture(cam, GP_CAPTURE_IMAGE, &camera_file_path, context);
    int fd = open(filename, O_CREAT | O_WRONLY, 0644);
    gp_file_new_from_fd(&file, fd);
    gp_camera_file_get(cam, camera_file_path.folder, camera_file_path.name, GP_FILE_TYPE_NORMAL, file, context);
    gp_camera_file_delete(cam, camera_file_path.folder, camera_file_path.name, context);
    
    mat = cv::imread(filename);
    if (mat.empty())
    {
        return false;
    }

    return true;
}

ReflexCam::~ReflexCam()
{
    if (context)
    {
        gp_context_unref(context);
    }
    if (cam)
    {
        gp_camera_unref(cam);
    }
}
