#include <lunasvg.h>
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>

static PyTypeObject Bitmap_Type = { PyVarObject_HEAD_INIT(nullptr, 0) };
static PyTypeObject Matrix_Type = { PyVarObject_HEAD_INIT(nullptr, 0) };
static PyTypeObject Box_Type = { PyVarObject_HEAD_INIT(nullptr, 0) };
static PyTypeObject Element_Type = { PyVarObject_HEAD_INIT(nullptr, 0) };
static PyTypeObject Document_Type = { PyVarObject_HEAD_INIT(nullptr, 0) };

typedef struct {
    PyObject_HEAD
    PyObject* data;
    lunasvg::Bitmap bitmap;
} Bitmap_Object;

static PyObject* Bitmap_Create(PyObject* data, lunasvg::Bitmap bitmap)
{
    Bitmap_Object* bitmap_ob = PyObject_New(Bitmap_Object, &Bitmap_Type);
    new (&bitmap_ob->bitmap) lunasvg::Bitmap(std::move(bitmap));
    bitmap_ob->data = data;
    Py_XINCREF(bitmap_ob->data);
    return (PyObject*)bitmap_ob;
}

static PyObject* Bitmap__new__(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    int width, height;
    if(!PyArg_ParseTuple(args, "ii:Bitmap.__init__", &width, &height))
        return nullptr;
    lunasvg::Bitmap bitmap(width, height);
    if(bitmap.isNull()) {
        PyErr_SetString(PyExc_MemoryError, "out of memory");
        return nullptr;
    }

    return Bitmap_Create(nullptr, std::move(bitmap));
}

static void Bitmap__del__(Bitmap_Object* self)
{
    self->bitmap.~Bitmap();
    Py_XDECREF(self->data);
    Py_TYPE(self)->tp_free(self);
}

static PyObject* Bitmap_create_for_data(PyTypeObject* type, PyObject* args)
{
    PyObject* data;
    int width, height, stride;
    if(!PyArg_ParseTuple(args, "Oiii", &data, &width, &height, &stride))
        return nullptr;
    Py_buffer buffer;
    if(PyObject_GetBuffer(data, &buffer, PyBUF_WRITABLE) == -1)
        return nullptr;
    if(height * stride > buffer.len) {
        PyBuffer_Release(&buffer);
        PyErr_SetString(PyExc_ValueError, "buffer is not long enough");
        return nullptr;
    }

    lunasvg::Bitmap bitmap((uint8_t*)buffer.buf, width, height, stride);
    if(bitmap.isNull()) {
        PyBuffer_Release(&buffer);
        PyErr_SetString(PyExc_MemoryError, "out of memory");
        return nullptr;
    }

    PyObject* bitmap_ob = Bitmap_Create(nullptr, std::move(bitmap));
    PyBuffer_Release(&buffer);
    return bitmap_ob;
}

static PyObject* Bitmap_data(Bitmap_Object* self, PyObject* args)
{
    return PyMemoryView_FromObject((PyObject*)self);
}

static PyObject* Bitmap_width(Bitmap_Object* self, PyObject* args)
{
    return PyLong_FromLong(self->bitmap.width());
}

static PyObject* Bitmap_height(Bitmap_Object* self, PyObject* args)
{
    return PyLong_FromLong(self->bitmap.width());
}

static PyObject* Bitmap_stride(Bitmap_Object* self, PyObject* args)
{
    return PyLong_FromLong(self->bitmap.stride());
}

static PyObject* Bitmap_clear(Bitmap_Object* self, PyObject* args)
{
    unsigned int color;
    if(!PyArg_ParseTuple(args, "I", &color)) {
        return nullptr;
    }

    Py_BEGIN_ALLOW_THREADS
    self->bitmap.clear(color);
    Py_END_ALLOW_THREADS
    Py_RETURN_NONE;
}

static PyObject* Bitmap_write_to_png(Bitmap_Object* self, PyObject* args)
{
    PyObject* file_ob;
    if(!PyArg_ParseTuple(args, "O&", PyUnicode_FSConverter, &file_ob)) {
        return nullptr;
    }

    bool success = false;
    Py_BEGIN_ALLOW_THREADS
    success = self->bitmap.writeToPng(PyBytes_AS_STRING(file_ob));
    Py_END_ALLOW_THREADS
    Py_DECREF(file_ob);
    if(!success) {
        PyErr_SetString(PyExc_IOError, "Failed to write PNG file.");
        return nullptr;
    }

    Py_RETURN_NONE;
}

static void stream_write_func(void* closure, void* data, int length)
{
    PyGILState_STATE gstate = PyGILState_Ensure();
    PyObject* result = PyObject_CallFunction((PyObject*)closure, "(y#)", data, length);
    Py_XDECREF(result);
    PyGILState_Release(gstate);
}

static int stream_write_conv(PyObject* ob, PyObject** target)
{
    if(PyObject_HasAttrString(ob, "write")) {
        PyObject* write_method = PyObject_GetAttrString(ob, "write");
        if(write_method && PyCallable_Check(write_method)) {
            *target = write_method;
            return 1;
        }

        Py_XDECREF(write_method);
    }

    PyErr_SetString(PyExc_TypeError, "stream must have a \"write\" method");
    return 0;
}

static PyObject* Bitmap_write_to_png_stream(Bitmap_Object* self, PyObject* args)
{
    PyObject* write_ob;
    if(!PyArg_ParseTuple(args, "O&", stream_write_conv, &write_ob)) {
        return nullptr;
    }

    bool success = false;
    Py_BEGIN_ALLOW_THREADS
    success = self->bitmap.writeToPng(stream_write_func, write_ob);
    Py_END_ALLOW_THREADS
    Py_DECREF(write_ob);
    if(!success) {
        PyErr_SetString(PyExc_IOError, "Failed to write PNG stream.");
        return nullptr;
    }

    Py_RETURN_NONE;
}

static int Bitmap__getbuffer__(Bitmap_Object* self, Py_buffer* view, int flags)
{
    void* data = self->bitmap.data();
    int height = self->bitmap.height();
    int stride = self->bitmap.stride();
    return PyBuffer_FillInfo(view, (PyObject*)self, data, height * stride, 0, flags);
}

static PyBufferProcs Bitmap_as_buffer = {
    (getbufferproc)Bitmap__getbuffer__,
    nullptr
};

static PyMethodDef Bitmap_methods[] = {
    {"create_for_data", (PyCFunction)Bitmap_create_for_data, METH_VARARGS | METH_CLASS},
    {"data", (PyCFunction)Bitmap_data, METH_NOARGS},
    {"width", (PyCFunction)Bitmap_width, METH_NOARGS},
    {"height", (PyCFunction)Bitmap_height, METH_NOARGS},
    {"stride", (PyCFunction)Bitmap_stride, METH_NOARGS},
    {"clear", (PyCFunction)Bitmap_clear, METH_VARARGS},
    {"write_to_png", (PyCFunction)Bitmap_write_to_png, METH_VARARGS},
    {"write_to_png_stream", (PyCFunction)Bitmap_write_to_png_stream, METH_VARARGS},
    {nullptr}
};

typedef struct {
    PyObject_HEAD
    lunasvg::Matrix matrix;
} Matrix_Object;

PyObject* Matrix_Create(lunasvg::Matrix matrix)
{
    Matrix_Object* matrix_ob = PyObject_New(Matrix_Object, &Matrix_Type);
    matrix_ob->matrix = matrix;
    return (PyObject*)matrix_ob;
}

static PyObject* Matrix__new__(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    lunasvg::Matrix matrix;
    if(!PyArg_ParseTuple(args, "|ffffff:Matrix.__init__", &matrix.a, &matrix.b, &matrix.c, &matrix.d, &matrix.e, &matrix.f))
        return nullptr;
    return Matrix_Create(matrix);
}

static void Matrix__del__(Matrix_Object* self)
{
    Py_TYPE(self)->tp_free(self);
}

static PyObject* Matrix__repr__(Matrix_Object* self)
{
    char buf[256];
    PyOS_snprintf(buf, sizeof(buf), "lunasvg.Matrix(%g, %g, %g, %g, %g, %g)",
        self->matrix.a, self->matrix.b, self->matrix.c, self->matrix.d, self->matrix.e, self->matrix.f);
    return PyUnicode_FromString(buf);
}

static PyObject* Matrix__mul__(Matrix_Object* self, PyObject* other)
{
    if(Py_TYPE(self) != Py_TYPE(other))
        Py_RETURN_NOTIMPLEMENTED;
    return Matrix_Create(self->matrix * ((Matrix_Object*)other)->matrix);
}

static PyObject* Matrix__imul__(Matrix_Object* self, PyObject* other)
{
    if(Py_TYPE(self) != Py_TYPE(other))
        Py_RETURN_NOTIMPLEMENTED;
    self->matrix *= ((Matrix_Object*)other)->matrix;
    Py_INCREF(self);
    return (PyObject*)self;
}

static PyObject* Matrix__invert__(Matrix_Object* self)
{
    return Matrix_Create(self->matrix.inverse());
}

static Py_ssize_t Matrix__len__(Matrix_Object* self)
{
    return 6;
}

static PyObject* Matrix__getitem__(Matrix_Object* self, Py_ssize_t index)
{
    switch(index) {
    case 0:
        return PyFloat_FromDouble(self->matrix.a);
    case 1:
        return PyFloat_FromDouble(self->matrix.b);
    case 2:
        return PyFloat_FromDouble(self->matrix.c);
    case 3:
        return PyFloat_FromDouble(self->matrix.d);
    case 4:
        return PyFloat_FromDouble(self->matrix.e);
    case 5:
        return PyFloat_FromDouble(self->matrix.f);
    default:
        PyErr_SetString(PyExc_IndexError, "Matrix index out of range");
        return nullptr;
    }
}

static PyObject* Matrix_multiply(Matrix_Object* self, PyObject* args)
{
    Matrix_Object* matrix_ob;
    if(!PyArg_ParseTuple(args, "|O!", &Matrix_Type, &matrix_ob)) {
        return nullptr;
    }

    self->matrix.multiply(matrix_ob->matrix);
    Py_INCREF(self);
    return (PyObject*)self;
}

static PyObject* Matrix_translate(Matrix_Object* self, PyObject* args)
{
    float tx, ty;
    if(!PyArg_ParseTuple(args, "ff", &tx, &ty)) {
        return nullptr;
    }

    self->matrix.translate(tx, ty);
    Py_INCREF(self);
    return (PyObject*)self;
}

static PyObject* Matrix_scale(Matrix_Object* self, PyObject* args)
{
    float sx, sy;
    if(!PyArg_ParseTuple(args, "ff", &sx, &sy)) {
        return nullptr;
    }

    self->matrix.scale(sx, sy);
    Py_INCREF(self);
    return (PyObject*)self;
}

static PyObject* Matrix_rotate(Matrix_Object* self, PyObject* args)
{
    float angle, cx = 0.f, cy = 0.f;
    if(!PyArg_ParseTuple(args, "f|ff", &angle, &cx, &cy)) {
        return nullptr;
    }

    self->matrix.rotate(angle, cx, cy);
    Py_INCREF(self);
    return (PyObject*)self;
}

static PyObject* Matrix_shear(Matrix_Object* self, PyObject* args)
{
    float shx, shy;
    if(!PyArg_ParseTuple(args, "ff", &shx, &shy)) {
        return nullptr;
    }

    self->matrix.shear(shx, shy);
    Py_INCREF(self);
    return (PyObject*)self;
}

static PyObject* Matrix_translated(PyTypeObject* type, PyObject* args)
{
    float tx, ty;
    if(!PyArg_ParseTuple(args, "ff", &tx, &ty)) {
        return nullptr;
    }

    return Matrix_Create(lunasvg::Matrix::translated(tx, ty));
}

static PyObject* Matrix_scaled(PyTypeObject* type, PyObject* args)
{
    float sx, sy;
    if(!PyArg_ParseTuple(args, "ff", &sx, &sy)) {
        return nullptr;
    }

    return Matrix_Create(lunasvg::Matrix::scaled(sx, sy));
}

static PyObject* Matrix_rotated(PyTypeObject* type, PyObject* args)
{
    float angle, cx = 0.f, cy = 0.f;
    if(!PyArg_ParseTuple(args, "f|ff", &angle, &cx, &cy)) {
        return nullptr;
    }

    return Matrix_Create(lunasvg::Matrix::rotated(angle, cx, cy));
}

static PyObject* Matrix_sheared(PyTypeObject* type, PyObject* args)
{
    float shx, shy;
    if(!PyArg_ParseTuple(args, "ff", &shx, &shy)) {
        return nullptr;
    }

    return Matrix_Create(lunasvg::Matrix::sheared(shx, shy));
}

static PyObject* Matrix_invert(Matrix_Object* self, PyObject* args)
{
    self->matrix.invert();
    Py_INCREF(self);
    return (PyObject*)self;
}

static PyObject* Matrix_inverse(Matrix_Object* self, PyObject* args)
{
    return Matrix_Create(self->matrix.inverse());
}

static PyObject* Matrix_reset(Matrix_Object* self, PyObject* args)
{
    self->matrix.reset();
    Py_RETURN_NONE;
}

static PyMethodDef Matrix_methods[] = {
    {"multiply", (PyCFunction)Matrix_multiply, METH_VARARGS},
    {"translate", (PyCFunction)Matrix_translate, METH_VARARGS},
    {"scale", (PyCFunction)Matrix_scale, METH_VARARGS},
    {"rotate", (PyCFunction)Matrix_rotate, METH_VARARGS},
    {"shear", (PyCFunction)Matrix_shear, METH_VARARGS},
    {"translated", (PyCFunction)Matrix_translated, METH_VARARGS| METH_CLASS},
    {"scaled", (PyCFunction)Matrix_scaled, METH_VARARGS | METH_CLASS},
    {"rotated", (PyCFunction)Matrix_rotated, METH_VARARGS | METH_CLASS},
    {"sheared", (PyCFunction)Matrix_sheared, METH_VARARGS| METH_CLASS},
    {"invert", (PyCFunction)Matrix_invert, METH_NOARGS},
    {"inverse", (PyCFunction)Matrix_inverse, METH_NOARGS},
    {"reset", (PyCFunction)Matrix_reset, METH_NOARGS},
    {nullptr}
};

static PyMemberDef Matrix_members[] = {
    {"a", T_FLOAT, offsetof(Matrix_Object, matrix.a), 0, nullptr},
    {"b", T_FLOAT, offsetof(Matrix_Object, matrix.b), 0, nullptr},
    {"c", T_FLOAT, offsetof(Matrix_Object, matrix.c), 0, nullptr},
    {"d", T_FLOAT, offsetof(Matrix_Object, matrix.d), 0, nullptr},
    {"e", T_FLOAT, offsetof(Matrix_Object, matrix.e), 0, nullptr},
    {"f", T_FLOAT, offsetof(Matrix_Object, matrix.f), 0, nullptr},
    {nullptr}
};

typedef struct {
    PyObject_HEAD
    lunasvg::Box box;
} Box_Object;

PyObject* Box_Create(lunasvg::Box box)
{
    Box_Object* box_ob = PyObject_New(Box_Object, &Box_Type);
    box_ob->box = box;
    return (PyObject*)box_ob;
}

static PyObject* Box__new__(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    lunasvg::Box box;
    if(!PyArg_ParseTuple(args, "|ffff:Box.__init__", &box.x, &box.y, &box.w, &box.h))
        return nullptr;
    return Box_Create(box);
}

static void Box__del__(Box_Object* self)
{
    Py_TYPE(self)->tp_free(self);
}

static PyObject* Box__repr__(Box_Object* self)
{
    char buf[256];
    PyOS_snprintf(buf, sizeof(buf), "lunasvg.Box(%g, %g, %g, %g)", self->box.x, self->box.y, self->box.w, self->box.h);
    return PyUnicode_FromString(buf);
}

static PyObject* Box_transform(Box_Object* self, PyObject* args)
{
    Matrix_Object* matrix_ob;
    if(!PyArg_ParseTuple(args, "|O!", &Matrix_Type, &matrix_ob)) {
        return nullptr;
    }

    self->box.transform(matrix_ob->matrix);
    Py_INCREF(self);
    return (PyObject*)self;
}

static PyObject* Box_transformed(Box_Object* self, PyObject* args)
{
    Matrix_Object* matrix_ob;
    if(!PyArg_ParseTuple(args, "|O!", &Matrix_Type, &matrix_ob)) {
        return nullptr;
    }

    return Box_Create(self->box.transformed(matrix_ob->matrix));
}

static Py_ssize_t Box__len__(Box_Object* self)
{
    return 4;
}

static PyObject* Box__getitem__(Box_Object* self, Py_ssize_t index)
{
    switch(index) {
    case 0:
        return PyFloat_FromDouble(self->box.x);
    case 1:
        return PyFloat_FromDouble(self->box.y);
    case 2:
        return PyFloat_FromDouble(self->box.w);
    case 3:
        return PyFloat_FromDouble(self->box.h);
    default:
        PyErr_SetString(PyExc_IndexError, "Box index out of range");
        return nullptr;
    }
}

static PyMethodDef Box_methods[] = {
    {"transform", (PyCFunction)Box_transform, METH_VARARGS},
    {"transformed", (PyCFunction)Box_transformed, METH_VARARGS},
    {nullptr}
};

static PyMemberDef Box_members[] = {
    {"x", T_FLOAT, offsetof(Box_Object, box.x), 0, nullptr},
    {"y", T_FLOAT, offsetof(Box_Object, box.y), 0, nullptr},
    {"w", T_FLOAT, offsetof(Box_Object, box.w), 0, nullptr},
    {"h", T_FLOAT, offsetof(Box_Object, box.h), 0, nullptr},
    {nullptr}
};

typedef struct {
    PyObject_HEAD
    PyObject* document_ob;
    lunasvg::Element element;
} Element_Object;

static PyObject* Element_Create(PyObject* document_ob, lunasvg::Element element)
{
    if(element.isNull())
        Py_RETURN_NONE;
    Element_Object* element_ob = PyObject_New(Element_Object, &Element_Type);
    element_ob->document_ob = document_ob;
    element_ob->element = element;
    Py_INCREF(element_ob->document_ob);
    return (PyObject*)element_ob;
}

static void Element__del__(Element_Object* self)
{
    Py_DECREF(self->document_ob);
    Py_TYPE(self)->tp_free(self);
}

static PyObject* Element__richcompare__(Element_Object* self, PyObject* other, int op)
{
    if(Py_TYPE(self) == Py_TYPE(other)) {
        Element_Object* element_ob = ((Element_Object*)other);
        if(op == Py_EQ) {
            if(self->element == element_ob->element)
                Py_RETURN_TRUE;
            Py_RETURN_FALSE;
        }

        if(op == Py_NE) {
            if(self->element != element_ob->element)
                Py_RETURN_TRUE;
            Py_RETURN_FALSE;
        }
    }

    Py_RETURN_NOTIMPLEMENTED;
}

static PyObject* Element_has_attribute(Element_Object* self, PyObject* args)
{
    const char* name;
    if(!PyArg_ParseTuple(args, "s", &name)) {
        return nullptr;
    }

    if(self->element.hasAttribute(name))
        Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}

static PyObject* Element_get_attribute(Element_Object* self, PyObject* args)
{
    const char* name;
    if(!PyArg_ParseTuple(args, "s", &name)) {
        return nullptr;
    }

    return PyUnicode_FromString(self->element.getAttribute(name).data());
}

static PyObject* Element_set_attribute(Element_Object* self, PyObject* args)
{
    const char* name;
    const char* value;
    if(!PyArg_ParseTuple(args, "ss", &name, &value)) {
        return nullptr;
    }

    Py_BEGIN_ALLOW_THREADS
    self->element.setAttribute(name, value);
    Py_END_ALLOW_THREADS
    Py_RETURN_NONE;
}

static PyObject* Element_render(Element_Object* self, PyObject* args)
{
    Bitmap_Object* bitmap_ob;
    Matrix_Object* matrix_ob = nullptr;
    if(!PyArg_ParseTuple(args, "O!|O!", &Bitmap_Type, &bitmap_ob, &Matrix_Type, &matrix_ob)) {
        return nullptr;
    }

    lunasvg::Matrix matrix;
    if(matrix_ob) {
        matrix = matrix_ob->matrix;
    }

    Py_BEGIN_ALLOW_THREADS
    self->element.render(bitmap_ob->bitmap, matrix);
    Py_END_ALLOW_THREADS
    Py_RETURN_NONE;
}

static PyObject* Element_render_to_bitmap(Element_Object* self, PyObject* args, PyObject* kwds)
{
    static const char* kwlist[] = { "width", "height", "background_color", nullptr };
    int width = -1, height = -1;
    unsigned int background_color = 0;
    if(!PyArg_ParseTupleAndKeywords(args, kwds, "|iiI", (char**)kwlist, &width, &height, &background_color)) {
        return nullptr;
    }

    lunasvg::Bitmap bitmap;
    Py_BEGIN_ALLOW_THREADS
    bitmap = self->element.renderToBitmap(width, height, background_color);
    Py_END_ALLOW_THREADS
    if(bitmap.isNull()) {
        PyErr_SetString(PyExc_ValueError, "invalid element size");
        return nullptr;
    }

    return Bitmap_Create(nullptr, std::move(bitmap));
}

static PyObject* Element_get_local_matrix(Element_Object* self, PyObject* args)
{
    return Matrix_Create(self->element.getLocalMatrix());
}

static PyObject* Element_get_global_matrix(Element_Object* self, PyObject* args)
{
    return Matrix_Create(self->element.getGlobalMatrix());
}

static PyObject* Element_get_local_bounding_box(Element_Object* self, PyObject* args)
{
    return Box_Create(self->element.getLocalBoundingBox());
}

static PyObject* Element_get_global_bounding_box(Element_Object* self, PyObject* args)
{
    return Box_Create(self->element.getGlobalBoundingBox());
}

static PyObject* Element_get_bounding_box(Element_Object* self, PyObject* args)
{
    return Box_Create(self->element.getBoundingBox());
}

static PyObject* Element_parent_element(Element_Object* self, PyObject* args)
{
    return Element_Create(self->document_ob, self->element.parentElement());
}

static PyObject* Element_owner_document(Element_Object* self, PyObject* args)
{
    Py_INCREF(self->document_ob);
    return self->document_ob;
}

static PyMethodDef Element_methods[] = {
    {"has_attribute", (PyCFunction)Element_has_attribute, METH_VARARGS},
    {"get_attribute", (PyCFunction)Element_get_attribute, METH_VARARGS},
    {"set_attribute", (PyCFunction)Element_set_attribute, METH_VARARGS},
    {"render", (PyCFunction)Element_render, METH_VARARGS},
    {"render_to_bitmap", (PyCFunction)Element_render_to_bitmap, METH_VARARGS | METH_KEYWORDS},
    {"get_local_matrix", (PyCFunction)Element_get_local_matrix, METH_NOARGS},
    {"get_global_matrix", (PyCFunction)Element_get_global_matrix, METH_NOARGS},
    {"get_local_bounding_box", (PyCFunction)Element_get_local_bounding_box, METH_NOARGS},
    {"get_global_bounding_box", (PyCFunction)Element_get_global_bounding_box, METH_NOARGS},
    {"get_bounding_box", (PyCFunction)Element_get_bounding_box, METH_NOARGS},
    {"parent_element", (PyCFunction)Element_parent_element, METH_NOARGS},
    {"owner_document", (PyCFunction)Element_owner_document, METH_NOARGS},
    {nullptr}
};

typedef struct {
    PyObject_HEAD
    std::unique_ptr<lunasvg::Document> document;
} Document_Object;

static PyObject* Document_Create(std::unique_ptr<lunasvg::Document> document)
{
    Document_Object* document_ob = PyObject_New(Document_Object, &Document_Type);
    new (&document_ob->document) std::unique_ptr<lunasvg::Document>(std::move(document));
    return (PyObject*)document_ob;
}

static PyObject* Document__new__(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    PyObject* file_ob;
    if(!PyArg_ParseTuple(args, "O&:Document.__init__", PyUnicode_FSConverter, &file_ob)) {
        return nullptr;
    }

    std::unique_ptr<lunasvg::Document> document;
    Py_BEGIN_ALLOW_THREADS
    document = lunasvg::Document::loadFromFile(PyBytes_AS_STRING(file_ob));
    Py_END_ALLOW_THREADS
    Py_DECREF(file_ob);
    if(document == nullptr) {
        PyErr_SetString(PyExc_ValueError, "Failed to load document from file.");
        return nullptr;
    }

    return Document_Create(std::move(document));
}

static void Document__del__(Document_Object* self)
{
    self->document.~unique_ptr<lunasvg::Document>();
    Py_TYPE(self)->tp_free(self);
}

static PyObject* Document_load_from_data(PyTypeObject* type, PyObject* args)
{
    const char* data;
    if(!PyArg_ParseTuple(args, "s", &data))
        return nullptr;
    std::unique_ptr<lunasvg::Document> document;
    Py_BEGIN_ALLOW_THREADS
    document = lunasvg::Document::loadFromData(data);
    Py_END_ALLOW_THREADS
    if(document == nullptr) {
        PyErr_SetString(PyExc_ValueError, "Failed to load document from data.");
        return nullptr;
    }

    return Document_Create(std::move(document));
}

static PyObject* Document_width(Document_Object* self, PyObject* args)
{
    return PyFloat_FromDouble(self->document->width());
}

static PyObject* Document_height(Document_Object* self, PyObject* args)
{
    return PyFloat_FromDouble(self->document->height());
}

static PyObject* Document_bounding_box(Document_Object* self, PyObject* args)
{
    return Box_Create(self->document->boundingBox());
}

static PyObject* Document_update_layout(Document_Object* self, PyObject* args)
{
    Py_BEGIN_ALLOW_THREADS
    self->document->updateLayout();
    Py_END_ALLOW_THREADS
    Py_RETURN_NONE;
}

static PyObject* Document_render(Document_Object* self, PyObject* args)
{
    Bitmap_Object* bitmap_ob;
    Matrix_Object* matrix_ob = nullptr;
    if(!PyArg_ParseTuple(args, "O!|O!", &Bitmap_Type, &bitmap_ob, &Matrix_Type, &matrix_ob)) {
        return nullptr;
    }

    lunasvg::Matrix matrix;
    if(matrix_ob) {
        matrix = matrix_ob->matrix;
    }

    Py_BEGIN_ALLOW_THREADS
    self->document->render(bitmap_ob->bitmap, matrix);
    Py_END_ALLOW_THREADS
    Py_RETURN_NONE;
}

static PyObject* Document_render_to_bitmap(Document_Object* self, PyObject* args, PyObject* kwds)
{
    static const char* kwlist[] = { "width", "height", "background_color", nullptr };
    int width = -1, height = -1;
    unsigned int background_color = 0;
    if(!PyArg_ParseTupleAndKeywords(args, kwds, "|iiI", (char**)kwlist, &width, &height, &background_color)) {
        return nullptr;
    }

    lunasvg::Bitmap bitmap;
    Py_BEGIN_ALLOW_THREADS
    bitmap = self->document->renderToBitmap(width, height, background_color);
    Py_END_ALLOW_THREADS
    if(bitmap.isNull()) {
        PyErr_SetString(PyExc_ValueError, "invalid document size");
        return nullptr;
    }

    return Bitmap_Create(nullptr, std::move(bitmap));
}

static PyObject* Document_get_element_by_id(Document_Object* self, PyObject* args)
{
    const char* id;
    if(!PyArg_ParseTuple(args, "s", &id)) {
        return nullptr;
    }

    lunasvg::Element element;
    Py_BEGIN_ALLOW_THREADS
    element = self->document->getElementById(id);
    Py_END_ALLOW_THREADS
    return Element_Create((PyObject*)self, element);
}

static PyObject* Document_document_element(Document_Object* self, PyObject* args)
{
    return Element_Create((PyObject*)self, self->document->documentElement());
}

static PyMethodDef Document_methods[] = {
    {"load_from_data", (PyCFunction)Document_load_from_data, METH_VARARGS | METH_CLASS},
    {"width", (PyCFunction)Document_width, METH_NOARGS},
    {"height", (PyCFunction)Document_height, METH_NOARGS},
    {"bounding_box", (PyCFunction)Document_bounding_box, METH_NOARGS},
    {"update_layout", (PyCFunction)Document_update_layout, METH_NOARGS},
    {"render", (PyCFunction)Document_render, METH_VARARGS},
    {"render_to_bitmap", (PyCFunction)Document_render_to_bitmap, METH_VARARGS | METH_KEYWORDS},
    {"get_element_by_id", (PyCFunction)Document_get_element_by_id, METH_VARARGS},
    {"document_element", (PyCFunction)Document_document_element, METH_NOARGS},
    {nullptr}
};

static PyObject* module_add_font_face_from_file(PyObject* self, PyObject* args)
{
    const char* family;
    PyObject* bold_ob;
    PyObject* italic_ob;
    PyObject* file_ob;
    if(!PyArg_ParseTuple(args, "sO!O!O&", &family, &PyBool_Type, &bold_ob, &PyBool_Type, &italic_ob, PyUnicode_FSConverter, &file_ob)) {
        return nullptr;
    }

    const bool bold = PyObject_IsTrue(bold_ob);
    const bool italic = PyObject_IsTrue(italic_ob);

    bool success = false;
    Py_BEGIN_ALLOW_THREADS
    success = lunasvg_add_font_face_from_file(family, bold, italic, PyBytes_AS_STRING(file_ob));
    Py_END_ALLOW_THREADS
    Py_DECREF(file_ob);
    if(!success) {
        PyErr_SetString(PyExc_ValueError, "Failed to add font face from file.");
        return nullptr;
    }

    Py_RETURN_NONE;
}

static void buffer_destroy_func(void* data)
{
    Py_buffer* buffer = (Py_buffer*)data;
    PyBuffer_Release(buffer);
    PyMem_Free(buffer);
}

static PyObject* module_add_font_face_from_data(PyObject* self, PyObject* args)
{
    const char* family;
    PyObject* bold_ob;
    PyObject* italic_ob;
    Py_buffer* buffer = (Py_buffer*)PyMem_Malloc(sizeof(Py_buffer));
    if(!PyArg_ParseTuple(args, "sO!O!y*", &family, &PyBool_Type, &bold_ob, &PyBool_Type, &italic_ob, buffer)) {
        PyMem_Free(buffer);
        return nullptr;
    }

    const bool bold = PyObject_IsTrue(bold_ob);
    const bool italic = PyObject_IsTrue(italic_ob);

    bool success = false;
    Py_BEGIN_ALLOW_THREADS
    success = lunasvg_add_font_face_from_data(family, bold, italic, buffer->buf, buffer->len, buffer_destroy_func, buffer);
    Py_END_ALLOW_THREADS
    if(!success) {
        PyErr_SetString(PyExc_ValueError, "Failed to add font face from data.");
        return nullptr;
    }

    Py_RETURN_NONE;
}

static PyMethodDef module_methods[] = {
    {"add_font_face_from_file", (PyCFunction)module_add_font_face_from_file, METH_VARARGS},
    {"add_font_face_from_data", (PyCFunction)module_add_font_face_from_data, METH_VARARGS},
    {nullptr}
};

static struct PyModuleDef module_definition = {
    PyModuleDef_HEAD_INIT,
    "lunasvg",
    0,
    0,
    module_methods,
    0,
    0,
    0,
    0,
};

PyMODINIT_FUNC PyInit__lunasvg(void)
{
    static PyNumberMethods Matrix_as_number = {0};
    Matrix_as_number.nb_multiply = (binaryfunc)Matrix__mul__;
    Matrix_as_number.nb_invert = (unaryfunc)Matrix__invert__;
    Matrix_as_number.nb_inplace_multiply = (binaryfunc)Matrix__imul__;

    static PySequenceMethods Matrix_as_sequence = {0};
    Matrix_as_sequence.sq_length = (lenfunc)Matrix__len__;
    Matrix_as_sequence.sq_item = (ssizeargfunc)Matrix__getitem__;

    Matrix_Type.tp_name = "lunasvg.Matrix";
    Matrix_Type.tp_basicsize = sizeof(Matrix_Object);
    Matrix_Type.tp_dealloc = (destructor)Matrix__del__;
    Matrix_Type.tp_repr = (reprfunc)Matrix__repr__;
    Matrix_Type.tp_as_number = &Matrix_as_number;
    Matrix_Type.tp_as_sequence = &Matrix_as_sequence;
    Matrix_Type.tp_flags = Py_TPFLAGS_DEFAULT;
    Matrix_Type.tp_methods = Matrix_methods;
    Matrix_Type.tp_members = Matrix_members;
    Matrix_Type.tp_new = (newfunc)Matrix__new__;
    if(PyType_Ready(&Matrix_Type) < 0) {
        return nullptr;
    }

    static PySequenceMethods Box_as_sequence = {0};
    Box_as_sequence.sq_length = (lenfunc)Box__len__;
    Box_as_sequence.sq_item = (ssizeargfunc)Box__getitem__;

    Box_Type.tp_name = "lunasvg.Box",
    Box_Type.tp_basicsize = sizeof(Box_Object);
    Box_Type.tp_dealloc = (destructor)Box__del__;
    Box_Type.tp_repr = (reprfunc)Box__repr__;
    Box_Type.tp_as_sequence = &Box_as_sequence;
    Box_Type.tp_flags = Py_TPFLAGS_DEFAULT;
    Box_Type.tp_methods = Box_methods;
    Box_Type.tp_members = Box_members;
    Box_Type.tp_new = (newfunc)Box__new__;
    if(PyType_Ready(&Box_Type) < 0) {
        return nullptr;
    }

    Bitmap_Type.tp_name = "lunasvg.Bitmap";
    Bitmap_Type.tp_basicsize = sizeof(Bitmap_Object);
    Bitmap_Type.tp_dealloc = (destructor)Bitmap__del__;
    Bitmap_Type.tp_as_buffer = &Bitmap_as_buffer;
    Bitmap_Type.tp_flags = Py_TPFLAGS_DEFAULT;
    Bitmap_Type.tp_methods = Bitmap_methods;
    Bitmap_Type.tp_new = (newfunc)Bitmap__new__;
    if(PyType_Ready(&Bitmap_Type) < 0) {
        return nullptr;
    }

    Element_Type.tp_name = "lunasvg.Element";
    Element_Type.tp_basicsize = sizeof(Element_Object);
    Element_Type.tp_dealloc = (destructor)Element__del__;
    Element_Type.tp_flags = Py_TPFLAGS_DEFAULT;
    Element_Type.tp_richcompare = (richcmpfunc)Element__richcompare__;
    Element_Type.tp_methods = Element_methods;
    if(PyType_Ready(&Element_Type) < 0) {
        return nullptr;
    }

    Document_Type.tp_name = "lunasvg.Document";
    Document_Type.tp_basicsize = sizeof(Document_Object);
    Document_Type.tp_dealloc = (destructor)Document__del__;
    Document_Type.tp_flags = Py_TPFLAGS_DEFAULT;
    Document_Type.tp_methods = Document_methods;
    Document_Type.tp_new = (newfunc)Document__new__;
    if(PyType_Ready(&Document_Type) < 0) {
        return nullptr;
    }

    PyObject* module = PyModule_Create(&module_definition);
    if(module == nullptr) {
        return nullptr;
    }

    Py_INCREF(&Matrix_Type);
    Py_INCREF(&Box_Type);
    Py_INCREF(&Bitmap_Type);
    Py_INCREF(&Element_Type);
    Py_INCREF(&Document_Type);

    PyModule_AddObject(module, "Matrix", (PyObject*)&Matrix_Type);
    PyModule_AddObject(module, "Box", (PyObject*)&Box_Type);
    PyModule_AddObject(module, "Bitmap", (PyObject*)&Bitmap_Type);
    PyModule_AddObject(module, "Element", (PyObject*)&Element_Type);
    PyModule_AddObject(module, "Element", (PyObject*)&Element_Type);
    PyModule_AddObject(module, "Document", (PyObject*)&Document_Type);

    PyModule_AddIntConstant(module, "LUNASVG_VERSION", LUNASVG_VERSION);
    PyModule_AddIntConstant(module, "LUNASVG_VERSION_MINOR", LUNASVG_VERSION_MINOR);
    PyModule_AddIntConstant(module, "LUNASVG_VERSION_MICRO", LUNASVG_VERSION_MICRO);
    PyModule_AddIntConstant(module, "LUNASVG_VERSION_MAJOR", LUNASVG_VERSION_MAJOR);
    PyModule_AddStringConstant(module, "LUNASVG_VERSION_STRING", LUNASVG_VERSION_STRING);

    PyModule_AddStringConstant(module, "version", LUNASVG_VERSION_STRINGIZE(PYLUNASVG_VERSION_MAJOR, PYLUNASVG_VERSION_MINOR, PYLUNASVG_VERSION_MICRO));
    PyModule_AddObject(module, "version_info", Py_BuildValue("(iii)", PYLUNASVG_VERSION_MAJOR, PYLUNASVG_VERSION_MINOR, PYLUNASVG_VERSION_MICRO));
    return module;
}
