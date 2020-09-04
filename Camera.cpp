/*
* Camera.cpp, 8/19/2020 9:15 PM
*/

static std::map<string, int> camera_offsets {
    {"width",      0x8},
    {"height",     0xc},
    {"matrix",    0xa8},
    {"position", 0x120},
};

class Camera : public RemoteMemoryObject {
public:


    Camera(addrtype address) : RemoteMemoryObject(address, &camera_offsets) {
        center_x = read<int>("width") / 2;
        center_y = read<int>("height") / 2;
    }

    Vector3& transform(Vector3& vec) {
        Matrix4x4 mat = read<Matrix4x4>("matrix");
        float x = vec.x * mat[0][0] + vec.y * mat[1][0] + vec.z * mat[2][0] + mat[3][0];
        float y = vec.x * mat[0][1] + vec.y * mat[1][1] + vec.z * mat[2][1] + mat[3][1];
        float z = vec.x * mat[0][2] + vec.y * mat[1][2] + vec.z * mat[2][2] + mat[3][2];
        float w = vec.x * mat[0][3] + vec.y * mat[1][3] + vec.z * mat[2][3] + mat[3][3];

        vec.x = (1.0 + x / w) * center_x;
        vec.y = (1.0 - y / w) * center_y;
        vec.z = z / w;

        return vec;
    }

    Vector3 position() {
        return read<Vector3>("position");
    }
};
