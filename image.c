#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <Imlib2.h>
#include <string.h>
#define MAX_WIDTH 640	//Maximum width in pixels for default display of image
#define MAX_HEIGHT 480	//Maximum height in pixels for default display of image
#define MAX_LINES 40	//Number of '\n' needed to make the shell prompt to go to the
						//bottom of the screen after clearing
void usage(void)    //display usage
{
    printf("Basic usage: image image_file\nFor more options see \'man image\'\n");
}

int main(int argc, char ** argv)
{
    int fbfd = 0;
    int i,j,k=0;

    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    long int screensize = 0;

    DATA32 *fbp = 0;
    long int location = 0;
    int w,h;
    DATA32 *data;
    Imlib_Image image, resized_image;

    int NEW_WIDTH=MAX_WIDTH, NEW_HEIGHT=MAX_HEIGHT;
    int posx=0, posy=0;
    int options=0, original=0;

    for(i=1; i<argc; i++)   //'options' is used to find the argument which holds the file name
    {
     options+=i;
    }
    for(i=1; i<argc; i++)    // go through all the arguments
    {
     if(argv[i][0]=='-')
     {
      if(strcmp(argv[i],"-s")==0)    //if -s option is specified
      {
       if(argc-i<3)     //see if there are two more arguments specifying the size
       {
         printf("Error: Size not specified properly\n");
         usage();
         return 1;
       }
       options -= (i+i+1+i+2);    //remove the size argument indices from the 'options'
       NEW_WIDTH = atoi(argv[i+1]);    //get the width and height for resizing
       NEW_HEIGHT = atoi(argv[i+2]);
      }
      else if(strcmp(argv[i],"-p")==0)    //if -p option is specified
      {
       if(argc-i<3)    //see if there are two more arguments specifying the position
       {
        printf("Error: Position not specified properly\n");
        usage();
        return 1;
       }
       options -= (i+i+1+i+2);    //remove the position argument indices from the 'options'
       posx = atoi(argv[i+1]);    //get the starting x and y positions
       posy = atoi(argv[i+2]);
      }
      else if(strcmp(argv[i],"-o")==0)    //is -o option is specified
      {
       original = 1;    //use original size of the image
       options -= i;    //remove the 'original' argument index from the 'options'
      }
      else
      {
       printf("Unknown option %s\n",argv[i]);
       usage();
       return 1;
      }
     }
    }
    if(options == 0)    //if no remaining options
    {
         printf("Error: No image file specified\n");    //then no image file has been specified
         usage();
         return 1;
    }

    // Open the file for reading and writing
    fbfd = open("/dev/fb0", O_RDWR);
    if (fbfd == -1)
    {
        perror("Error: cannot open framebuffer device");
        exit(1);
    }

    // Get fixed screen information
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1)
    {
        perror("Error reading fixed information");
        exit(2);
    }

    // Get variable screen information
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1)
    {
        perror("Error reading variable information");
        exit(3);
    }

    // Figure out the size of the screen in bytes
    screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;

    // Map the device to memory
    fbp = (DATA32 *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if (fbp == NULL)
    {
        perror("Error: failed to map framebuffer device to memory");
        exit(4);
    }

    // Current implementation only supports 32 bits per pixel
    if(vinfo.bits_per_pixel != 32)
    {
        perror("Error: failed as the number of bits per pixel is not 32");
        exit(5);
    }

    image = imlib_load_image(argv[options]);    //load the image to virtual buffer from file
    if (image == NULL)
    {
      printf("Error: Can't load image\n");
      return 1;
    }

    imlib_context_set_image(image);    //set the context to the virtual buffer of the image
    w = imlib_image_get_width();     //get the width of the image
    h = imlib_image_get_height();    //get the height of the image
    if(original==1 && w<=MAX_HEIGHT && h<=MAX_HEIGHT)     //if 'original' option is specified
    {    //and the original size doesnot exceed the MAX values
        NEW_WIDTH = w;
        NEW_HEIGHT = h;
    }
    if((NEW_WIDTH + posx) > vinfo.xres || (NEW_HEIGHT + posy) > vinfo.yres)
    {
        printf("Error: Image exceeds the framebuffer resolution limits\n");
        return 1;
    }
    if( w!=NEW_WIDTH && h!=NEW_HEIGHT)    //if the new values are not the same as the old values of width and height
    {
        resized_image = imlib_create_cropped_scaled_image(0, 0, w, h, NEW_WIDTH, NEW_HEIGHT); //then resize the image
        if (resized_image == NULL)  //if unable to resize
        {
          printf("Error: Can't resize image\n");
          return 1;
        }
    }
    else   //if the new and old values are the same for width and height
    {
        resized_image = image;
    }
    imlib_free_image();    //free the original image virtual buffer
    imlib_context_set_image(resized_image);    //set context to resized image virtual buffer
    data = imlib_image_get_data_for_reading_only();   //get the pixel data from the image

    system("clear");    //clear the screen
    for(i=0;i<MAX_LINES;i++)    //take the shell prompt to lowest position in the screen
    {
      putchar('\n');
    }

    for(j=0;j<NEW_HEIGHT;j++)   //go through the image data pixels
    {
        for(i=0;i<NEW_WIDTH;i++)
	    {
	        location = (i+posx+vinfo.xoffset) * (vinfo.bits_per_pixel/32) + (j+posy+vinfo.yoffset)*(finfo.line_length/4);    //use the framebuffer location
	        k=i+(j*NEW_WIDTH);    //get the offset for the pixel value in pixel data memory
	        *(fbp+location)=*(data+k);    //set the framebuffer memory to the corresponding pixel value
 	    }
    }
    imlib_free_image();    //free the resized image`s virtual buffer

    munmap(fbp, screensize);   //unmap the memory given for framebuffer
    close(fbfd);   //close the framebuffer file

    return 0;
}
