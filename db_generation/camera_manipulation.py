import numpy as np
from math import cos, sin, radians
np.set_printoptions(precision=3, suppress=True)


def get_translation_matrix(t):
    return np.array(
        [[1, 0, 0, t[0]],
         [0, 1, 0, t[1]],
         [0, 0, 1, t[2]],
         [0, 0, 0, 1]]
    )


def get_rotation_x_axis(theta):
    theta = np.radians(theta)
    return np.array(
        [[1, 0, 0, 0],
         [0, np.cos(theta), -np.sin(theta), 0],
         [0, np.sin(theta), np.cos(theta), 0],
         [0, 0, 0, 1]], dtype=np.float32)


def get_rotation_y_axis(theta):
    theta = np.radians(theta)
    return np.array(
        [[np.cos(theta), 0, np.sin(theta), 0],
         [0, 1, 0, 0],
         [-np.sin(theta), 0, np.cos(theta), 0],
         [0, 0, 0, 1]], dtype=np.float32)


def get_rotation_z_axis(theta):
    theta = np.radians(theta)
    return np.array(
        [[np.cos(theta), -np.sin(theta), 0, 0],
         [np.sin(theta), np.cos(theta), 0, 0],
         [0, 0, 1, 0],
         [0, 0, 0, 1]], dtype=np.float32)


if __name__ == "__main__":
    print('Test for rotation from 0 to 90 degree angle')
    point_x = np.array([[1], [0], [1], [1]])
    point_y = np.array([[0], [1], [1], [1]])
    point_z = np.array([[1], [0], [1], [1]])
    x, y, z = (point_x, point_y, point_z)
    for i in range(10, 100, 10):
        x = np.append(x, np.matmul(get_rotation_x_axis(i), point_x), axis=1)
        y = np.append(y, np.matmul(get_rotation_y_axis(i), point_y), axis=1)
        z = np.append(z, np.matmul(get_rotation_z_axis(i), point_z), axis=1)
    print('x axis\n', x, '\ny axis\n', y, '\nz axis\n', z)
