#include<GL/glut.h>
//#include<cmath>
#include<math.h>
#include<stdlib.h>
#include <stdio.h>
#include<algorithm>
#include<iostream>
using namespace std;

int width=500;
int height=500;
float image[500][500][3];
bool IntSphere=true; 
bool IntTriangle=false; 
bool IntTetrahedron=true; 
bool IntSurface=true; 
int IntTriangle_No=0;

struct Color
{
	 float red,green,blue;
};

struct Vector
{
	double x,y,z;
};

struct Sphere
{
	Vector O;
	double R;
	Color c;
};

struct Triangle
{
	Vector ver1,ver2,ver3;
	Vector normal;
};

struct Tetrahedron
{
	Triangle t1,t2,t3,t4;
	Color c;
};

struct Surface
{
	Vector x;
	Vector normal;
};

struct Intensity
{
	double red,green,blue;
};

struct Light
{
	Intensity Ia,Ib;
	Color ka,kd,ks;
};

Vector LookAt,LigDir;
Sphere sphere1,sphere2;
Light light1;
Tetrahedron T;
Intensity cl,cr;
Color ca;
Surface sur;
double alpha=0;

Vector subtract(Vector a,Vector b)
{
	Vector result;
	result.x=a.x-b.x;
	result.y=a.y-b.y;
	result.z=a.z-b.z;
	return result;
}

Vector Add(Vector a,Vector b)
{
	Vector result;
	result.x=a.x+b.x;
	result.y=a.y+b.y;
	result.z=a.z+b.z;
	return result;
}

double multiply(Vector a,Vector b)
{
	double result;
	result=a.x*b.x+a.y*b.y+a.z*b.z;
	return result;
}

double getLength(Vector v)
{
	double result;
	result=sqrt(v.x*v.x+v.y*v.y+v.z*v.z);
	return result;
}

double getDistance(Vector p1,Vector p2)
{
	double result;
	result=sqrt((p1.x-p2.x)*(p1.x-p2.x)+(p1.y-p2.y)*(p1.y-p2.y)+(p1.z-p2.z)*(p1.z-p2.z));
	return result;
}

Vector getUnit(Vector point)
{
	Vector result;
	double len;
	len=getLength(point);
	result.x=point.x/len;
	result.y=point.y/len;
	result.z=point.z/len;
	return result;
}

Vector GetCrossproduct(Vector a,Vector b)
{
	Vector result;
	result.x=a.y*b.z-a.z*b.y;//a ¡Á b= [a2b3-a3b2,a3b1-a1b3, a1b2-a2b1]
	result.y=a.z*b.x-a.x*b.z;
	result.z=a.x*b.y-a.y*b.x;
	return result;
}

Vector getNormal_Sphere(Vector interpoint,Sphere s)
{
	Vector result;
	result.x=interpoint.x-s.O.x;
	result.y=interpoint.y-s.O.y;
	result.z=interpoint.z-s.O.z;
	result=getUnit(result);
	return result;
}

Vector getNormal_Triangle(int Triangle_num)
{
	Vector result;
	switch(Triangle_num)
	{
		case 1:result=T.t1.normal;break;
		case 2:result=T.t2.normal;break;
		case 3:result=T.t3.normal;break;
		case 4:result=T.t4.normal;break;
		default:result.x=result.y=result.z=0;
	}
	return result;
}

Vector GetInterPoint_Sphere(Vector pixel,Vector dir,Sphere s)
{
	IntSphere=true;
	Vector result;
	result.x=result.y=result.z=0;
	double temp1,temp2,t1,t2,t;
	temp1=multiply(dir,subtract(pixel,s.O));//d*(e-c)
	temp1=temp1*temp1;
	temp2=multiply(dir,dir)*((multiply(subtract(pixel,s.O),subtract(pixel,s.O))-s.R*s.R));//(D`D)((e-c)(e-c)-R2)
	if((temp1-temp2)<0)
		IntSphere=false;
	else
	{
		temp1=sqrt(temp1-temp2);
		temp2=-(multiply(dir,subtract(pixel,s.O)));//-d(e-c)
		t1=(temp2+temp1)/multiply(dir,dir);
		t2=(temp2-temp1)/multiply(dir,dir);
		t=fabs(t1)>fabs(t2)?t2:t1;
		result.x=pixel.x+t*dir.x;//e+td
		result.y=pixel.y+t*dir.y;
		result.z=pixel.z+t*dir.z;
	}
	return result;
}

Vector GetInterPoint_Surface(Vector pixel,Vector dir,Surface s)
{
	IntSurface=true;
	double t1;
	Vector result;
	result.x=result.y=result.z=0;
	
	t1=(multiply(s.x,s.normal)-multiply(pixel,s.normal))/multiply(dir,s.normal);
	if(t1<-2||t1>5)
		IntSurface=false;
	else
	{
		result.x=pixel.x+t1*dir.x;//e+td
		result.y=pixel.y+t1*dir.y;
		result.z=pixel.z+t1*dir.z;
	}
	return result;
}

Vector GetInterPoint_Triangle(Vector pixel,Vector dir,Triangle t)
{
	IntTriangle=false;
	double t1;
	Vector result,temp;
	result.x=result.y=result.z=0;
	
	t1=multiply(subtract(t.ver1,pixel),t.normal)/multiply(dir,t.normal);
	
	Vector ab=subtract(t.ver2,t.ver1);
	Vector bc=subtract(t.ver3,t.ver2);
	Vector ca=subtract(t.ver1,t.ver3);

	temp.x=pixel.x+t1*dir.x;//e+td
	temp.y=pixel.y+t1*dir.y;
	temp.z=pixel.z+t1*dir.z;

	if(multiply(GetCrossproduct(ab,subtract(temp,t.ver1)),t.normal)>0)
		if(multiply(GetCrossproduct(bc,subtract(temp,t.ver2)),t.normal)>0)
			if(multiply(GetCrossproduct(ca,subtract(temp,t.ver3)),t.normal)>0)
			{
				IntTriangle=true;
				result=temp;
			}
	return result;
}

Vector GetInterPoint_Tetrahedron(Vector pixel,Vector dir,Tetrahedron t)
{
	IntTetrahedron=true;
	IntTriangle_No=0;
	double distance_temp,distance=10;
	Vector result_temp,result;
	result.x=result.y=result.z=0;
	result_temp=GetInterPoint_Triangle(pixel,dir,t.t1);
	if(IntTriangle==true)
	{
		result=result_temp;
		distance_temp=getDistance(pixel,result);
		distance=distance_temp;
		IntTriangle_No=1;
	}
	result_temp=GetInterPoint_Triangle(pixel,dir,t.t2);
	if(IntTriangle==true)
	{
		distance_temp=getDistance(pixel,result_temp);
		if(distance_temp<distance)
		{
			result=result_temp;
			distance=distance_temp;
			IntTriangle_No=2;
		}
	}
	result_temp=GetInterPoint_Triangle(pixel,dir,t.t3);
	if(IntTriangle==true)
	{
		distance_temp=getDistance(pixel,result_temp);
		if(distance_temp<distance)
		{
			result=result_temp;
			distance=distance_temp;
			IntTriangle_No=3;
		}
	}
	
	result_temp=GetInterPoint_Triangle(pixel,dir,t.t4);
	if(IntTriangle==true)
	{
		distance_temp=getDistance(pixel,result_temp);
		if(distance_temp<distance)
		{
			result=result_temp;
			distance=distance_temp;
			IntTriangle_No=4;
		}
	}
	if(IntTriangle_No==0)
		IntTetrahedron=false;
	return result;
}

Color get_pixel_color(Vector ligdir,Vector viewdir,Vector normal,Light I)
{
	Color L;
	Vector h;//biosector
	h=getUnit(Add(viewdir,ligdir));

	double mul_nl,mul_nh,max_nl,max_nh; 
	mul_nl=multiply(ligdir,normal);
	mul_nh=multiply(normal,h);
	max_nl=mul_nl>0?mul_nl:0;
	max_nh=mul_nh>0?mul_nh:0;
	
	L.red=I.ka.red*I.Ia.red+I.kd.red*I.Ib.red*max_nl+I.ks.red*I.Ib.red*pow(max_nh,30);
	L.green=I.ka.green*I.Ia.green+I.kd.green*I.Ib.green*max_nl+I.ks.green*I.Ib.green*pow(max_nh,30);
	L.blue=I.ka.blue*I.Ia.blue+I.kd.blue*I.Ib.blue*max_nl+I.ks.blue*I.Ib.blue*pow(max_nh,30);
	return L;
}

Vector get_Projection(Vector dir,Vector normal)
{
	Vector r;
	dir.x=-dir.x;
	dir.y=-dir.y;
	dir.z=-dir.z;
	r.x=2*multiply(dir,normal)*normal.x;
	r.y=2*multiply(dir,normal)*normal.y;
	r.z=2*multiply(dir,normal)*normal.z;
	r=subtract(dir,r);
	return r;
}

void Init_Light()
{
	LigDir.x=-0.3;
	LigDir.y=0.5;
	LigDir.z=0.6;
	LigDir=getUnit(LigDir);
	
	light1.ka.red=0.6;//surface's ambient coefficient
	light1.ka.green=0.6;
	light1.ka.blue=0.6;
	
	light1.kd.red=0.6;//diffuse coefficient
	light1.kd.green=0.6;
	light1.kd.blue=0.6;

	light1.ks.red=1;//specular coefficient
	light1.ks.green=1;
	light1.ks.blue=1;

	light1.Ia.red=0.5;//surface's ambient light intensity
	light1.Ia.green=0.5;
	light1.Ia.blue=0.5;
	light1.Ib.red=1;//light source intensity
	light1.Ib.green=1;
	light1.Ib.blue=1;
}

void Init_Sphere1()
{
	sphere1.O.x=-0.6;
	sphere1.O.y=0.2;
	sphere1.O.z=0.7;
	sphere1.R=0.3;

	sphere1.c.red=0.9;
	sphere1.c.green=0.2;
	sphere1.c.blue=0.1;
}

void Init_Sphere2()
{
	sphere2.O.x=0;
	sphere2.O.y=0.1;
	sphere2.O.z=0.5;
	sphere2.R=0.2;

	sphere2.c.red=0.2;
	sphere2.c.green=0.1;
	sphere2.c.blue=0.8;
}

Triangle Init_Triangle(Vector a,Vector b,Vector c)
{
	Triangle tr;
	tr.ver1=a;
	tr.ver2=b;
	tr.ver3=c;

	Vector b_a,b_c;

	b_a=subtract(tr.ver2,tr.ver1);
	b_c=subtract(tr.ver3,tr.ver2);
	
	tr.normal=getUnit(GetCrossproduct(b_a,b_c));

	return tr;
}

void Init_Tetrahedron()
{
	Vector a,b,c,d;
	a.x=0.6;
	a.y=0.3;
	a.z=0.2;
	
	b.x=0.3;
	b.y=-0.1;
	b.z=0.1;

	c.x=0.5;
	c.y=-0.1;
	c.z=0.3;

	d.x=0.9;
	d.y=-0.1;
	d.z=0;

	T.t1=Init_Triangle(a,b,c);
	T.t2=Init_Triangle(a,b,d);
	T.t3=Init_Triangle(a,c,d);
	T.t4=Init_Triangle(b,c,d);

	T.c.red=0.1;
	T.c.green=0.8;
	T.c.blue=0.1;
}

void Init_Surface()
{
	sur.normal.x=0;
	sur.normal.y=1;
	sur.normal.z=0;
	sur.normal=getUnit(sur.normal);
	sur.x.x=-0.6;
	sur.x.y=-0.1;
	sur.x.z=0;
}

/*Just display an image*/
void myDisplay1(void)
{
	int x,y;
	for (x=0;x<500;x++)
		for(y=0;y<500;y++)
		{
			if(x<=250&&y<=250)
			{
				image[y][x][0] =0.3;
				image[y][x][1] =0.3;
				image[y][x][2] =0;
			}
			else if(x<=250&&y>=250)
			{
				image[y][x][0] =0;
				image[y][x][1] =0.3;
				image[y][x][2] =0.3;
			}
			else if(x>=250&&y<=250)
			{
				image[y][x][0] =0.3;
				image[y][x][1] =0;
				image[y][x][2] =0.3;
			}
			else
			{
				image[y][x][0] =0;
				image[y][x][1] =0;
				image[y][x][2] =0;
			}	
		}
	glClearColor(0,0,1,0);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawPixels(width,height,GL_RGB,GL_FLOAT,image);	
	glutSwapBuffers();
}

/*Ray Tracing*/
void myDisplay2(void)
{
	int x,y;
	double x_d,y_d;
	Vector pixel,interpoint1,interpoint2,interpoint3;
	//Color pixelcolor;
	LookAt.x=sin(alpha);
	LookAt.y=0.3;
	LookAt.z=cos(alpha);
	LookAt=getUnit(LookAt);
	for (x=0;x<500;x++)
		for(y=0;y<500;y++)
		{
			x_d=double(x);
			y_d=double(y);
			pixel.x=(x_d-250)/250*cos(alpha)+sin(alpha);
			pixel.y=(y_d-250)/250;
			pixel.z=cos(alpha)-(x_d-250)/250*sin(alpha);//pixel's location

			image[y][x][0] =0.5;
			image[y][x][1] =0.5;
			image[y][x][2] =0.5;
			
			interpoint1=GetInterPoint_Sphere(pixel,LookAt,sphere1);
			if(IntSphere==true)//draw sphere1
			{
				image[y][x][0] =0.8;
				image[y][x][1] =0;
				image[y][x][2] =0;
				
			}	
			interpoint2=GetInterPoint_Sphere(pixel,LookAt,sphere2);
			if(IntSphere==true)//draw Sphere2
			{
				image[y][x][0] =0;
				image[y][x][1] =0;
				image[y][x][2] =0.8;
			}
			interpoint3=GetInterPoint_Tetrahedron(pixel,LookAt,T);
			if(IntTetrahedron==true)//draw tetrahedron
			{
				image[y][x][0] =0;
				image[y][x][1] =0.8;
				image[y][x][2] =0;
			}
		}
	glClearColor(0,0,1,0);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawPixels(width,height,GL_RGB,GL_FLOAT,image);	
	glutSwapBuffers();
}//

/*Shading*/
void myDisplay3(void)
{
	int x,y;
	double x_d,y_d;
	Vector pixel,interpoint1,interpoint2,interpoint3,normal;
	double d1,d2,d3,d;
	Color pixelcolor;
	LookAt.x=sin(alpha);
	LookAt.y=0.3;
	LookAt.z=cos(alpha);
	LookAt=getUnit(LookAt);
	for (x=0;x<500;x++)
		for(y=0;y<500;y++)
		{
			d=10;
			x_d=double(x);
			y_d=double(y);
			pixel.x=(x_d-250)/250*cos(alpha)+sin(alpha);
			pixel.y=(y_d-250)/250;
			pixel.z=cos(alpha)-(x_d-250)/250*sin(alpha);//pixel's location

			image[y][x][0] =0.5;
			image[y][x][1] =0.5;
			image[y][x][2] =0.5;
			
			interpoint1=GetInterPoint_Sphere(pixel,LookAt,sphere1);
			if(IntSphere==true)//draw sphere1
			{
				d1=getDistance(pixel,interpoint1);
				if(d1<d)
				{
					d=d1;
					normal=getNormal_Sphere(interpoint1,sphere1);
					pixelcolor=get_pixel_color(LigDir,LookAt,normal,light1);
					image[y][x][0] =pixelcolor.red*sphere1.c.red;
					image[y][x][1] =pixelcolor.green*sphere1.c.green;
					image[y][x][2] =pixelcolor.blue*sphere1.c.blue;
				}
			}	
			interpoint2=GetInterPoint_Sphere(pixel,LookAt,sphere2);
			if(IntSphere==true)//draw Sphere2
			{
				d2=getDistance(pixel,interpoint2);
				if(d2<d)
				{
					d=d2;
					normal=getNormal_Sphere(interpoint2,sphere2);
					pixelcolor=get_pixel_color(LigDir,LookAt,normal,light1);
					image[y][x][0] =pixelcolor.red*sphere2.c.red;
					image[y][x][1] =pixelcolor.green*sphere2.c.green;
					image[y][x][2] =pixelcolor.blue*sphere2.c.blue;
				}
			}
			interpoint3=GetInterPoint_Tetrahedron(pixel,LookAt,T);
			if(IntTetrahedron==true)//draw tetrahedron
			{
				d3=getDistance(pixel,interpoint3);
				if(d3<d)
				{
					d=d3;
					normal=getNormal_Triangle(IntTriangle_No);
					pixelcolor=get_pixel_color(LigDir,LookAt,normal,light1);
					image[y][x][0] =pixelcolor.red*T.c.red;
					image[y][x][1] =pixelcolor.green*T.c.green;
					image[y][x][2] =pixelcolor.blue*T.c.blue;
				}
			}
		}
	glClearColor(0,0,1,0);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawPixels(width,height,GL_RGB,GL_FLOAT,image);	
	glutSwapBuffers();
}

/*glazed surface and animation*/
void myDisplay4(void)
{
	int x,y;
	double x_d,y_d;
	Vector pixel,interpoint0,interpoint1,interpoint2,interpoint3,normal,ligtemp,pro_dir;
	double d0,d1,d2,d3,d;
	Color pixelcolor;
	LookAt.x=sin(alpha);
	LookAt.y=0.3;
	LookAt.z=cos(alpha);
	LookAt=getUnit(LookAt);
	for (x=0;x<500;x++)
		for(y=0;y<500;y++)
		{
			d=10;
			x_d=double(x);
			y_d=double(y);
			pixel.x=(x_d-250)/250*cos(alpha)+sin(alpha);
			pixel.y=(y_d-250)/250;
			pixel.z=cos(alpha)-(x_d-250)/250*sin(alpha);//pixel's location

			/*Draw the surface*/
			interpoint0=GetInterPoint_Surface(pixel,LookAt,sur);
			if(IntSurface==false)//Background
			{
				image[y][x][0] =0.8;
				image[y][x][1] =0.8;
				image[y][x][2] =0.8;
			}
			else
			{
				d0=getDistance(pixel,interpoint0);
				d=d0;
				image[y][x][0] =0.5;//color of the surface
				image[y][x][1] =0.5;
				image[y][x][2] =0.5;
			
				pro_dir=get_Projection(LookAt,sur.normal);
				pro_dir.x=-pro_dir.x;
				pro_dir.y=-pro_dir.y;
				pro_dir.z=-pro_dir.z;
				interpoint1=GetInterPoint_Sphere(interpoint0,pro_dir,sphere1);//projection of sphere 1
				if(IntSphere==true)
				{
					normal=getNormal_Sphere(interpoint1,sphere1);
					pixelcolor=get_pixel_color(LigDir,LookAt,normal,light1);
					image[y][x][0] =pixelcolor.red*sphere1.c.red*0.5;
					image[y][x][1] =pixelcolor.green*sphere1.c.green*0.5;
					image[y][x][2] =pixelcolor.blue*sphere1.c.blue*0.5;
				}
				interpoint2=GetInterPoint_Sphere(interpoint0,pro_dir,sphere2);//projection of sphere 2
				if(IntSphere==true)
				{
					normal=getNormal_Sphere(interpoint2,sphere2);
					pixelcolor=get_pixel_color(LigDir,LookAt,normal,light1);
					image[y][x][0] =pixelcolor.red*sphere2.c.red*0.5;
					image[y][x][1] =pixelcolor.green*sphere2.c.green*0.5;
					image[y][x][2] =pixelcolor.blue*sphere2.c.blue*0.5;
				}
				interpoint3=GetInterPoint_Tetrahedron(interpoint0,pro_dir,T);
				if(IntTetrahedron==true)//projection of tetrahedron
				{
					normal=getNormal_Triangle(IntTriangle_No);
					pixelcolor=get_pixel_color(LigDir,LookAt,normal,light1);
					image[y][x][0] =pixelcolor.red*T.c.red*0.5;
					image[y][x][1] =pixelcolor.green*T.c.green*0.5;
					image[y][x][2] =pixelcolor.blue*T.c.blue*0.5;
				}
				ligtemp.x=-LigDir.x;
				ligtemp.y=-LigDir.y;
				ligtemp.z=-LigDir.z;
				interpoint1=GetInterPoint_Sphere(interpoint0,ligtemp,sphere1);
				if(IntSphere==true)//in shadow of sphere1
				{
					image[y][x][0]= light1.ka.red*light1.Ia.red;//ka*Ia
					image[y][x][1]= light1.ka.green*light1.Ia.green;
					image[y][x][2]= light1.ka.blue*light1.Ia.blue;
				}
				interpoint2=GetInterPoint_Sphere(interpoint0,LigDir,sphere2);
				if(IntSphere==true)//in shadow of sphere2
				{
					image[y][x][0]= light1.ka.red*light1.Ia.red;//ka*Ia
					image[y][x][1]= light1.ka.green*light1.Ia.green;
					image[y][x][2]= light1.ka.blue*light1.Ia.blue;
				}
				interpoint3=GetInterPoint_Tetrahedron(interpoint0,ligtemp,T);
				if(IntTetrahedron==true)//in shadow of tetrahedron
				{
					image[y][x][0]= light1.ka.red*light1.Ia.red;//ka*Ia
					image[y][x][1]= light1.ka.green*light1.Ia.green;
					image[y][x][2]= light1.ka.blue*light1.Ia.blue;
				}
			}
			interpoint1=GetInterPoint_Sphere(pixel,LookAt,sphere1);
			if(IntSphere==true)//draw sphere1
			{
				d1=getDistance(pixel,interpoint1);
				if(d1<d)
				{
					d=d1;
					normal=getNormal_Sphere(interpoint1,sphere1);
					pixelcolor=get_pixel_color(LigDir,LookAt,normal,light1);
					image[y][x][0] =pixelcolor.red*sphere1.c.red;
					image[y][x][1] =pixelcolor.green*sphere1.c.green;
					image[y][x][2] =pixelcolor.blue*sphere1.c.blue;
				}
			}	
			interpoint2=GetInterPoint_Sphere(pixel,LookAt,sphere2);
			if(IntSphere==true)//draw Sphere2
			{
				d2=getDistance(pixel,interpoint2);
				if(d2<d)
				{
					d=d2;
					normal=getNormal_Sphere(interpoint2,sphere2);
					pixelcolor=get_pixel_color(LigDir,LookAt,normal,light1);
					image[y][x][0] =pixelcolor.red*sphere2.c.red;
					image[y][x][1] =pixelcolor.green*sphere2.c.green;
					image[y][x][2] =pixelcolor.blue*sphere2.c.blue;
				}
			}
			interpoint3=GetInterPoint_Tetrahedron(pixel,LookAt,T);
			if(IntTetrahedron==true)//draw tetrahedron
			{
				d3=getDistance(pixel,interpoint3);
				if(d3<d)
				{
					d=d3;
					normal=getNormal_Triangle(IntTriangle_No);
					pixelcolor=get_pixel_color(LigDir,LookAt,normal,light1);
					image[y][x][0] =pixelcolor.red*T.c.red;
					image[y][x][1] =pixelcolor.green*T.c.green;
					image[y][x][2] =pixelcolor.blue*T.c.blue;
				}
			}
		}
	glClearColor(0,0,1,0);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawPixels(width,height,GL_RGB,GL_FLOAT,image);	

	alpha=alpha+0.3;
	if(alpha>6.28)
	{
		alpha=0;
	}
	glutPostRedisplay();
	glutSwapBuffers();
}

void Display(void)
{
	int choice=1;
	cout<<endl<<endl<<endl<<endl<<endl<<endl<<endl<<endl<<endl;
	cout<<"    ---------------------------------Menu---------------------------------    "<<endl;
	cout<<"                             1:Display an image                            "<<endl;
	cout<<"                             2:Ray tracing                                 "<<endl;
	cout<<"                             3:Shading                                     "<<endl;
	cout<<"                             4:Glazed surface and animation                "<<endl;
	cin>>choice;
	switch(choice)
	{
		case 1:
		{
			glutDisplayFunc(&myDisplay1);
			glutMainLoop();
		}
		break;
		case 2:
		{
			glutDisplayFunc(&myDisplay2);
			glutMainLoop();
		}
		break;
		case 3:
		{
			glutDisplayFunc(&myDisplay3);
			glutMainLoop();
		}
		break;
		case 4:
		{
			glutDisplayFunc(&myDisplay4);
			glutMainLoop();
		}
		break;
		default:exit(0);
	}
}

int main(int argc,char *argv[])
{	
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE);
    glutInitWindowPosition(100,100);
    glutInitWindowSize(width,height);
	Init_Light();
	Init_Sphere1();
	Init_Sphere2();
	Init_Tetrahedron();
	Init_Surface();
    glutCreateWindow("Ray tracing");
	Display();
    return 0;
}
