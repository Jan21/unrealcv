import numpy as np
from math import cos, sin, radians
np.set_printoptions(precision=5, suppress=True)


def get_translation_matrix(t):
    return np.array(
        [[1, 0, 0, t[0]],
         [0, 1, 0, t[1]],
         [0, 0, 1, t[2]],
         [0, 0, 0, 1]]
    )


def get_rotation_x_axis(theta):
    theta = radians(theta)
    return np.array(
        [[1, 0, 0, 0],
         [0, cos(theta), -sin(theta), 0],
         [0, sin(theta), cos(theta), 0],
         [0, 0, 0, 1]]
    )


def get_rotation_y_axis(theta):
    theta = radians(theta)
    return np.array(
        [[cos(theta), 0, sin(theta), 0],
         [0, 1, 0, 0],
         [-sin(theta), 0, cos(theta), 0],
         [0, 0, 0, 1]]
    )


def get_rotation_z_axis(theta):
    theta = radians(theta)
    return np.array(
        [[cos(theta), -sin(theta), 0, 0],
         [sin(theta), cos(theta), 0, 0],
         [0, 0, 1, 0],
         [0, 0, 0, 1]]
    )


if __name__ == "__main__":
    point = np.array([[1], [1], [1], [1]])
    print(np.matmul(get_rotation_x_axis(90), point))
    print(np.matmul(get_rotation_y_axis(90), point))
    print(np.matmul(get_rotation_z_axis(90), point))
