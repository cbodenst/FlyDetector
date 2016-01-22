#include <fcntl.h>
#include <iostream>

#include "reflexcam.h"

ReflexCam::ReflexCam() : cam(nullptr), context(nullptr)
{
    int error;

    error = gp_camera_new(&this->cam);
    if (error != GP_OK)
    {
        this->accessable = false;
        return;
    }

    this->context = gp_context_new();
    if (this->context == nullptr)
    {
        this->accessable = false;
        return;
    }

    error = gp_camera_init(this->cam, this->context);
    if (error != GP_OK) {
        gp_camera_free(this->cam);
        this->cam = nullptr;
        this->accessable = false;
    }
    else
    {
        this->accessable = true;
    }
}

bool ReflexCam::getImage(cv::Mat& mat)
{
    const char filename[] = "temp";
    CameraFile* file;
    CameraFilePath camera_file_path;

    // TODO: what happens if camera is deconnected mid process?
    gp_camera_capture(cam, GP_CAPTURE_IMAGE, &camera_file_path, this->context);
    int fd = open(filename, O_CREAT | O_WRONLY, 0644);
    gp_file_new_from_fd(&file, fd);
    gp_camera_file_get(this->cam, camera_file_path.folder, camera_file_path.name, GP_FILE_TYPE_NORMAL, file, this->context);
    gp_camera_file_delete(this->cam, camera_file_path.folder, camera_file_path.name, this->context);
    
    mat = cv::imread(filename);
    if (mat.empty())
    {
        return false;
    }

    return true;
}

ReflexCam::~ReflexCam()
{
    if (this->context)
    {
        gp_context_unref(this->context);
    }
    if (this->cam)
    {
        gp_camera_unref(this->cam);
    }
}
