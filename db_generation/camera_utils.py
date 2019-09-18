import numpy as np
from unrealcv import client
from unrealcv.util import read_png

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


def rotate_around_object(obj_loc: tuple, d: tuple, angle: tuple, y_max: int, view_modes: list)-> dict:
    """
    Rotates camera around obj_loc and return images from 360 view around for each view mode. Client has to be connected.
    :param obj_loc: 3-tuple, coordinates of the object location (x,y,z)
    :param d: 2-tuple, distance of camera from obj_loc (distance on x axis, distance on z axis)
    :param angle: 2-tuple, angle step by which is camera rotated around z and y axis (z step, y step)
    :param y_max: top y angle value by which can camera be rotated above object
    :param view_modes: list of strings representing requested view modes. Allowed are lit, normal, depth, object_mask.
    :return: Dictionary with given view modes as keys and lists of images as values.
    """
    output = {}
    for view_mode in view_modes:
        output[view_mode] = []
    t = get_translation_matrix([-obj_loc[0], -obj_loc[1], -obj_loc[2]])
    t_neg = get_translation_matrix([obj_loc[0], obj_loc[1], obj_loc[2]])
    cam_start_loc = [obj_loc[0] - d[0], obj_loc[1], obj_loc[2] + d[1]]
    loc_orig = np.matmul(t, np.array([[cam_start_loc[0]], [cam_start_loc[1]], [cam_start_loc[2]], [1.0]]))
    h_rot = list(
        zip([get_rotation_z_axis(x) for x in range(0, 360, angle[0])], [x for x in range(0, 360, angle[0])]))
    y_angles = [y for y in range(0, -y_max, -angle[1])]
    v_rot = list(zip([get_rotation_y_axis(y) for y in range(0, len(y_angles) * angle[1], angle[1])], y_angles))
    for v_params in v_rot:
        for h_params in h_rot:
            new_loc = np.round(np.matmul(t_neg, np.matmul(h_params[0], np.matmul(v_params[0], loc_orig))))
            assert client.request('vset /camera/0/rotation {} {} {}'.format(v_params[1], h_params[1] % 360, 0)) == 'ok', \
                'Did not get \'ok\' response from urealcv server for setting camera rotation'
            assert client.request('vset /camera/0/location {} {} {}'.format(new_loc[0][0],
                                                                            new_loc[1][0],
                                                                            new_loc[2][0])) == 'ok', \
                'Did not get \'ok\' response from urealcv server for setting camera location'
            for view_mode in view_modes:
                output[view_mode].append(read_png(client.request('vget /camera/0/' + view_mode + ' png')))
    return output
	
	
def set_other_objects_visibility(id: str, visible: bool):
	"""
    Set objects not containing id in their name to invisible for 
	object mask mode
    :id For example sun in sunflower
	:visible If false, hide objects, if not show objects
    """
	objects = client.request('vget /objects')
	objects = objects.split()
	objects = [obj for obj in objects if id not in obj]
	for obj in objects:
		if visible:
			client.request('vset /object/' + obj + '/show')
		else:
			client.request('vset /object/' + obj + '/hide')


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
