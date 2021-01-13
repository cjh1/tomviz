import numpy as np

import tomviz.utils

from vtk import vtkImageData

_installed = False
try:
    from databroker import catalog
    _installed = True
except:
    pass

def installed():
    print("install")
    return _installed

def catalogs():
    return list(catalog)

def runs(catalog_name):
    runs = []
    for uid, run in catalog[catalog_name].items():
        runs.append({
            "uid": uid,
            "name": run.name,
            "time": run.metadata['start']['time']
        })

    return runs

def variables(catalog_name, table, run_uid):
    return list(catalog[catalog_name][run_uid][table].read().data_vars.keys())

def load_variable(catalog_name, run_uid, table, variable):
    data = catalog[catalog_name][run_uid][table].read()[variable].data
    shape = data.shape
    data = data.reshape((shape[0]*shape[1], shape[2], shape[3]))

    # Convert to Fortran ordering
    data = np.asfortranarray(data)

    image_data = vtkImageData()
    (x, y, z) = data.shape

    # Metadata?
    image_data.SetOrigin(0, 0, 0)
    image_data.SetSpacing(1, 1, 1)
    image_data.SetExtent(0, x - 1, 0, y - 1, 0, z - 1)
    tomviz.utils.set_array(image_data, data)

    return image_data

#print(variables("test", "primary", "1b0b4d73-6d87-43ab-8d62-ed035c51b9b4"))


