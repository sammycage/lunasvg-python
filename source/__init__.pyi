from __future__ import annotations
from typing import Type, Union, Optional, BinaryIO, Tuple
import os

version: str = ...
"""
Represents the version of this module as a string in the format "major.minor.micro".
"""

version_info: Tuple[int, int, int] = ...
"""
Represents the version of this module as a tuple of three integers: (major, minor, micro).
"""

LUNASVG_VERSION: int = ...
"""
Represents the version of the lunasvg library encoded as a single integer.
"""

LUNASVG_VERSION_MAJOR: int = ...
"""
Represents the major version number of the lunasvg library.
"""

LUNASVG_VERSION_MICRO: int = ...
"""
Represents the micro version number of the lunasvg library.
"""

LUNASVG_VERSION_MINOR: int = ...
"""
Represents the minor version number of the lunasvg library.
"""

LUNASVG_VERSION_STRING: str = ...
"""
Represents the version of the lunasvg library as a string in the format "major.minor.micro".
"""

class Bitmap:
    """
    The `Bitmap` class provides an interface for rendering to memory buffers.
    """

    def __init__(self, width: int, height: int) -> None:
        """
        Initializes a new `Bitmap` with the specified width and height.

        :param width: The width of the bitmap in pixels.
        :param height: The height of the bitmap in pixels.
        """

    @classmethod
    def create_for_data(cls, data: memoryview, width: int, height: int, stride: int) -> Bitmap:
        """
        Creates a new `Bitmap` for the given writable memoryview.

        :param data: A writable memoryview representing the bitmap data.
        :param width: The width of the bitmap in pixels.
        :param height: The height of the bitmap in pixels.
        :param stride: The number of bytes per row of the bitmap.
        :returns: A new `Bitmap` instance.
        """

    def data(self) -> memoryview:
        """
        Returns a writable memoryview representing the bitmap data.

        :returns: A writable memoryview representing the bitmap data.
        """

    def width(self) -> int:
        """
        Returns the width of the bitmap.

        :returns: The width of the bitmap in pixels.
        """

    def height(self) -> int:
        """
        Returns the height of the bitmap.

        :returns: The height of the bitmap in pixels.
        """

    def stride(self) -> int:
        """
        Returns the stride (bytes per row) of the bitmap.

        :returns: The number of bytes per row in the bitmap.
        """

    def clear(self, color: int) -> None:
        """
        Clears the bitmap with the specified color in 0xRRGGBBAA format.

        :param color: The color to fill the bitmap with, in 0xRRGGBBAA format.
        """

    def write_to_png(self, filename: Union[str, bytes, os.PathLike]) -> None:
        """
        Writes the bitmap to a PNG file.

        :param filename: The file path where the PNG should be written.
        """

    def write_to_png_stream(self, stream: BinaryIO) -> None:
        """
        Writes the bitmap to a PNG stream.

        :param stream: A writable binary stream to output the PNG.
        """

class Matrix:
    """
    The `Matrix` class represents a 2D transformation matrix.
    """
    def __init__(self, a: float = 1.0, b: float = 0.0, c: float = 0.0, d: float = 1.0, e: float = 0.0, f: float = 0.0) -> None:
        """
        Initializes the matrix with the specified values.

        :param a: The horizontal scaling factor.
        :param b: The vertical shearing factor.
        :param c: The horizontal shearing factor.
        :param d: The vertical scaling factor.
        :param e: The horizontal translation offset.
        :param f: The vertical translation offset.
        """

    def __repr__(self) -> str:
        """
        Returns a string representation of the matrix.

        :returns: A string that represents the matrix.
        """

    def __len__(self) -> int:
        """
        Returns the number of elements in the matrix.

        :returns: The number of elements in the matrix.
        """

    def __getitem__(self, index: int) -> float:
        """
        Retrieves the matrix element at the specified index.

        :param index: The index of the matrix element to retrieve.
        :returns: The matrix element at the given index.
        """

    def __mul__(self, other: Matrix) -> Matrix:
        """
        Multiplies this matrix with another matrix.

        :param other: The matrix to multiply with.
        :returns: The product of the two matrices.
        """

    def __imul__(self, other: Matrix) -> Matrix:
        """
        Multiplies this matrix with another matrix in-place.

        :param other: The matrix to multiply with.
        :returns: This matrix after multiplication.
        """

    def __invert__(self) -> Matrix:
        """
        Returns the inverse of this matrix.

        :returns: The inverse of this matrix.
        """

    def multiply(self, other: Matrix) -> Matrix:
        """
        Multiplies this matrix with another matrix.

        :param other: The matrix to multiply with.
        :returns: This matrix after multiplication.
        """

    def translate(self, tx: float, ty: float) -> Matrix:
        """
        Translates this matrix by the specified offsets.

        :param tx: The horizontal translation offset.
        :param ty: The vertical translation offset.
        :returns: This matrix after translation.
        """

    def scale(self, sx: float, sy: float) -> Matrix:
        """
        Scales this matrix by the specified factors.

        :param sx: The horizontal scaling factor.
        :param sy: The vertical scaling factor.
        :returns: This matrix after scaling.
        """

    def rotate(self, angle: float, tx: float = 0.0, ty: float = 0.0) -> Matrix:
        """
        Rotates this matrix by the specified angle around the specified point.

        :param angle: The rotation angle in degrees.
        :param tx: The x-coordinate of the rotation point.
        :param ty: The y-coordinate of the rotation point.
        :returns: This matrix after rotation.
        """

    def shear(self, shx: float, shy: float) -> Matrix:
        """
        Shears this matrix by the specified factors.

        :param shx: The horizontal shear factor.
        :param shy: The vertical shear factor.
        :returns: This matrix after shearing.
        """

    @classmethod
    def translated(cls, tx: float, ty: float) -> Matrix:
        """
        Creates a new translation matrix.

        :param tx: The horizontal translation offset.
        :param ty: The vertical translation offset.
        :returns: A new translation matrix.
        """

    @classmethod
    def scaled(cls, sx: float, sy: float) -> Matrix:
        """
        Creates a new scaling matrix.

        :param sx: The horizontal scaling factor.
        :param sy: The vertical scaling factor.
        :returns: A new scaling matrix.
        """

    @classmethod
    def rotated(cls, angle: float, tx: float = 0.0, ty: float = 0.0) -> Matrix:
        """
        Creates a new rotation matrix.

        :param angle: The rotation angle in degrees.
        :param tx: The x-coordinate of the rotation point.
        :param ty: The y-coordinate of the rotation point.
        :returns: A new rotation matrix.
        """
    def sheared(cls, shx: float, shy: float) -> Matrix:
        """
        Creates a new shearing matrix.

        :param shx: The horizontal shearing factor.
        :param shy: The vertical shearing factor.
        :returns: A new shearing matrix.
        """

    def invert(self) -> Matrix:
        """
        Inverts this matrix.

        :returns: This matrix after inversion.
        """

    def inverse(self) -> Matrix:
        """
        Returns the inverse of this matrix.

        :returns: The inverse of this matrix.
        """

    def reset(self) -> None:
        """
        Resets this matrix to the identity matrix.
        """

    a: float = ...
    """
    The horizontal scaling factor.
    """

    b: float = ...
    """
    The vertical shearing factor.
    """

    c: float = ...
    """
    The horizontal shearing factor.
    """

    d: float = ...
    """
    The vertical scaling factor.
    """

    e: float = ...
    """
    The horizontal translation offset.
    """

    f: float = ...
    """
    The vertical translation offset.
    """

class Box:
    """
    The `Box` class represents a 2D axis-aligned bounding box.
    """
    def __init__(self, x: float = 0.0, y: float = 0.0, w: float = 0.0, h: float = 1.0) -> None:
        """
        Initializes the box with the specified position and size.

        :param x: The x-coordinate of the box's origin.
        :param y: The y-coordinate of the box's origin.
        :param w: The width of the box.
        :param h: The height of the box.
        """

    def __repr__(self) -> str:
        """
        Returns a string representation of the box.

        :returns: A string that represents the box.
        """

    def __len__(self) -> int:
        """
        Returns the number of attributes in the box.

        :returns: The number of attributes in the box.
        """

    def __getitem__(self, index: int) -> float:
        """
        Retrieves the box attribute by index.

        :param index: The index of the attribute to retrieve.
        :returns: The attribute value at the given index.
        """

    def transform(self, matrix: Matrix) -> Box:
        """
        Transforms this box using the given matrix.

        :param matrix: The transformation matrix.
        :returns: This box after transformation.
        """

    def transformed(self, matrix: Matrix) -> Box:
        """
        Returns a new box transformed by the given matrix.

        :param matrix: The transformation matrix.
        :returns: A new box after transformation.
        """

    x: float = ...
    """
    The x-coordinate of the box's origin.
    """

    y: float = ...
    """
    The y-coordinate of the box's origin.
    """

    w: float = ...
    """
    The width of the box.
    """

    h: float = ...
    """
    The height of the box.
    """

class Element:
    """
    The `Element` class represents an SVG element.
    """
    def __eq__(self, other: object) -> bool:
        """
        Checks if this element is equal to another element.

        :param other: The element to compare with.
        :returns: `True` if the elements are equal, `False` otherwise.
        """

    def __ne__(self, other: object) -> bool:
        """
        Checks if this element is not equal to another element.

        :param other: The element to compare with.
        :returns: `True` if the elements are not equal, `False` otherwise.
        """

    def has_attribute(self, name: str) -> bool:
        """
        Checks if the element has a specified attribute.

        :param name: The name of the attribute.
        :returns: `True` if the attribute exists, `False` otherwise.
        """

    def get_attribute(self, name: str) -> str:
        """
        Retrieves the value of a specified attribute.

        :param name: The name of the attribute.
        :returns: The value of the attribute.
        """

    def set_attribute(self, name: str, value: str) -> None:
        """
        Sets the value of a specified attribute.

        :param name: The name of the attribute to set.
        :param value: The value to assign to the attribute.
        """

    def render(self, bitmap: Bitmap, matrix: Matrix = ...) -> None:
        """
        Renders the element onto a bitmap using a transformation matrix.

        :param bitmap: The target bitmap for rendering.
        :param matrix: The root transformation matrix.
        """

    def render_to_bitmap(self, width: int = -1, height: int = -1, background_color: int = 0x00000000) -> Bitmap:
        """
        Renders the element to a bitmap with specified dimensions.

        :param width: The desired width in pixels, or -1 to auto-scale based on the intrinsic size.
        :param height: The desired height in pixels, or -1 to auto-scale based on the intrinsic size.
        :param background_color: The background color in 0xRRGGBBAA format.
        :returns: A `Bitmap` containing the raster representation of the element.
        """

    def get_local_matrix(self) -> Matrix:
        """
        Returns the local transformation matrix of the element.

        :returns: The matrix that applies only to the element, relative to its parent.
        """

    def get_global_matrix(self) -> Matrix:
        """
        Returns the global transformation matrix of the element.

        :returns: The matrix combining the element's local and all parent transformations.
        """

    def get_local_bounding_box(self) -> Box:
        """
        Returns the local bounding box of the element.

        :returns: The bounding box after applying local transformations.
        """

    def get_global_bounding_box(self) -> Box:
        """
        Returns the global bounding box of the element.

        :returns: The bounding box after applying global transformations.
        """

    def get_bounding_box(self) -> Box:
        """
        Returns the untransformed bounding box of the element.

        :returns: The untransformed bounding box of the element.
        """

    def parent_element(self) -> Optional[Element]:
        """
        Returns the parent element of this element, or `None` if the element has no parent.

        :returns: The parent `Element` if it exists, otherwise `None`.
        """

    def owner_document(self) -> Document:
        """
        Returns the document to which this element is attached.

        :returns: The `Document` that owns this element.
        """

class Document:
    """
    The `Document` class represents an SVG document.
    """
    def __init__(self, filename: Union[str, bytes, os.PathLike]) -> None:
        """
        Loads an SVG document from a file.

        :param filename: The path to the SVG file.
        """

    @classmethod
    def load_from_data(cls, data: str) -> Document:
        """
        Loads an SVG document from a string.

        :param data: The string containing the SVG data.
        :returns: A `Document` instance containing the parsed SVG data.
        """

    def width(self) -> float:
        """
        Returns the intrinsic width of the document.

        :returns: The intrinsic width of the document in pixels.
        """

    def height(self) -> float:
        """
        Returns the intrinsic height of the document.

        :returns: The intrinsic height of the document in pixels.
        """

    def bounding_box(self) -> Box:
        """
        Returns the smallest rectangle that encloses the document content.

        :returns: A `Box` representing the bounding box of the document.
        """

    def update_layout(self) -> None:
        """
        Updates the layout of the document.
        """

    def render(self, bitmap: Bitmap, matrix: Matrix = ...) -> None:
        """
        Renders the document onto a bitmap using a transformation matrix.

        :param bitmap: The target bitmap for rendering.
        :param matrix: The root transformation matrix.
        """

    def render_to_bitmap(self, width: int = -1, height: int = -1, background_color: int = 0x00000000) -> None:
        """
        Renders the document to a bitmap with specified dimensions.

        :param width: The desired width in pixels, or -1 to auto-scale based on the intrinsic size.
        :param height: The desired height in pixels, or -1 to auto-scale based on the intrinsic size.
        :param background_color: The background color in 0xRRGGBBAA format.
        :returns: A `Bitmap` containing the raster representation of the document.
        """

    def get_element_by_id(self, id: str) -> Optional[Element]:
        """
        Retrieves an element from the document by its ID.

        :param id: The ID of the element to retrieve.
        :returns: The `Element` with the specified ID, or `None` if not found.
        """

    def document_element(self) -> Element:
        """
        Returns the root element of the document.

        :returns: The root `Element` of the document.
        """

def add_font_face_from_file(family: str, bold: bool, italic: bool, filename: Union[str, bytes, os.PathLike]) -> None:
    """
    Adds a font face to the font cache from a font file.

    :param family: The name of the font family.
    :param bold: A boolean indicating if the font is bold.
    :param italic: A boolean indicating if the font is italic.
    :param filename: The path to the font file.
    """

def add_font_face_from_data(family: str, bold: bool, italic: bool, data: memoryview) -> None:
    """
    Adds a font face to the font cache from font data.

    :param family: The name of the font family.
    :param bold: A boolean indicating if the font is bold.
    :param italic: A boolean indicating if the font is italic.
    :param data: A `memoryview` object containing the font data.
    """
