### Test Images from July 3, 2024

Sending over Iridium network was simulated by compressing raw image file (test_image.RAW) into messages (120 binary files stored in image_binary folder) and then expanding on other side and reconstructing the image from a 2D array of bytes (grayscale pixels) using Python Pillow module (see ../reconstruct_from_messages.ipynb). 

JPEG (test_image.JPG) was captured with the camera in the same location and orientation, at the same resolution as the RAW file (640x480). 

**Notes:**
- raw images (and by extension, reconstructed images) are grayscale, inverted (upside down) and only show 1/4 of the scene captured by the jpeg image