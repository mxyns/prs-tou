# Press the green button in the gutter to run the script.
import numpy as np
import matplotlib.pyplot as plt
from matplotlib import cm

d0 = [
    [1, 2, 4, 8, 16, 32, 24, 18, 10, 2],
    [2, 4, 8, 12, 16, 20, 10, 6, 4, 2],
    [3, 12, 10, 8, 6, 4, 2, 8, 6, 10],
    [4, 2, 4, 8, 10, 16, 32, 40, 20, 10],
    [5, 3, 2, 4, 4, 4, 10, 20, 50, 10],
    [5, 4, 3, 4, 1, 2, 13, 29, 46, 14],
    [4, 4, 2, 3, 1, 8, 13, 33, 56, 16],
    [3, 4, 1, 2, 4, 14, 12, 37, 66, 18],
    [2, 4, 0, 0, 7, 20, 12, 41, 75, 21],
    [1, 4, 1, 0, 10, 26, 11, 45, 85, 23],
]

d1 = [
    [1, 2, 4, 8, 16, 32, 24, 18, 10, 3*2],
    [1, 2, 4, 8, 16, 32, 24, 18, 10, 3*4],
    [1, 2, 4, 8, 16, 32, 24, 18, 10, 3*6],
    [1, 2, 4, 8, 16, 32, 24, 18, 10, 3*8],
    [1, 2, 4, 8, 16, 33, 24, 18, 10, 3*10],
    [1, 2, 4, 8, 16, 33, 24, 18, 10, 3*10],
    [1, 2, 4, 8, 16, 32, 24, 18, 10, 3*8],
    [1, 2, 4, 8, 16, 32, 24, 18, 10, 3*6],
    [1, 2, 4, 8, 16, 32, 24, 18, 10, 3*4],
    [1, 2, 4, 8, 16, 32, 24, 18, 10, 3*2],
]


def graph(x, y):
    plt.scatter(x, y, c=y, cmap="winter", edgecolor='none')
    plt.show()


def scaled_surface(scalex, scaley, data):
    X, Y = np.meshgrid(scalex, scaley)
    arr = np.array(data)

    ax = plt.axes(projection='3d')
    ax.plot_surface(X, Y, arr, rstride=1, cstride=1,
                    cmap='viridis', edgecolor='none')
    ax.set_title('surface')
    plt.show()


def surface(data):
    scalex = np.linspace(0, len(data[0]), len(data[0]), endpoint=False)
    scaley = np.linspace(0, len(data), len(data), endpoint=False)

    scaled_surface(scalex, scaley, data)


def maxof(x, y, data):
    arr = np.array(data)
    result = np.where(arr == np.amax(arr))
    result = list(zip(result[0], result[1]))
    return [((coord[1], coord[0]), (x[coord[1]], y[coord[0]]), data[coord[0]][coord[1]]) for coord in result]

    
def convert_points_map_to_arrays(datamap):
    Xs, Ys = zip(*datamap.keys())
    Xs = sorted(list(set(Xs)))
    Ys = sorted(list(set(Ys)))

    DimX = len(Xs)
    DimY = len(Ys)

    data = [[datamap.get((Xs[xi], Ys[yi])) for yi in range(DimY)] for xi in range(DimX)]
    data = [[data[yi][xi] if data[yi][xi] is not None else -1 for yi in range(DimY) ] for xi in range(DimX)]

    return Xs, Ys, data


def main():
    data = d1
    x = np.array([1, 2, 3, 4, 5, 6, 7, 8, 9, 10])
    y = np.array([10, 20, 30, 40, 50, 60, 70, 72, 74, 76])

    surface(data)
    scaled_surface(x, y, data)

    for mx in maxof(x, y, data):
        print(mx)


if __name__ == '__main__':
    main()