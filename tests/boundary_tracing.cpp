/**
 * boundary tracing
 * @author Tobias Weber
 * @date may-2021
 * @license see 'LICENSE.EUPL' file
 */

#include <iostream>
#include <memory>


template<class t_pixel=bool>
class Image
{
public:
	Image(std::size_t w, std::size_t h)
		: m_width{w}, m_height{h},
		m_img{std::make_unique<t_pixel[]>(w*h)}
	{
	}


	Image(const Image<t_pixel>& img)
		: Image(img.GetWidth(), img.GetHeight())
	{
		SetImage(img.m_img.get());
	}


	~Image()
	{
		m_img.reset();
	}


	std::size_t GetWidth() const
	{
		return m_width;
	}


	std::size_t GetHeight() const
	{
		return m_height;
	}


	t_pixel GetPixel(std::size_t x, std::size_t y) const
	{
		if(x < GetWidth() && y < GetHeight())
			return m_img[y*m_width + x];
		return t_pixel{};
	}


	void SetPixel(std::size_t x, std::size_t y, t_pixel pix)
	{
		if(x < GetWidth() && y < GetHeight())
			m_img[y*m_width + x] = pix;
	}


	void SetImage(const t_pixel* img)
	{
		for(std::size_t y = 0; y < GetHeight(); ++y)
		{
			for(std::size_t x = 0; x < GetWidth(); ++x)
			{
				SetPixel(x, y, img[y*GetWidth() + x]);
			}
		}
	}


private:
	std::size_t m_width{}, m_height{};
	std::unique_ptr<t_pixel[]> m_img;
};


template<class t_pixel>
std::ostream& operator<<(std::ostream& ostr, const Image<t_pixel>& img)
{
	for(std::size_t y = 0; y < img.GetHeight(); ++y)
	{
		for(std::size_t x = 0; x < img.GetWidth(); ++x)
			ostr << img.GetPixel(x, y);

		ostr << "\n";
	}

	return ostr;
}


/**
 * boundary tracing
 * @see http://www.imageprocessingplace.com/downloads_V3/root_downloads/tutorials/contour_tracing_Abeer_George_Ghuneim/ray.html
 */
template<class t_pixel>
Image<t_pixel> trace_boundary(const Image<t_pixel>& img)
{
	Image<t_pixel> boundary{img.GetWidth(), img.GetHeight()};


	// find start pixel
	std::size_t start[2] = {0, 0};
	bool start_found = 0;

	for(std::size_t y=0; y<img.GetHeight(); ++y)
	{
		for(std::size_t x=0; x<img.GetWidth(); ++x)
		{
			if(img.GetPixel(x, y))
			{
				start[0] = x;
				start[1] = y;
				start_found = 1;
				break;
			}

			if(start_found)
				break;
		}
	}

	if(!start_found)
		return boundary;
	else
		boundary.SetPixel(start[0], start[1], 1);


	// trace boundary
	std::size_t pos[2] = { start[0], start[1] };
	int dir[2] = {1, 0};
	int next_dir[2] = {0, 0};
	bool has_next_dir = false;

	// next possible position depending on direction
	auto get_next_dir = [](const int* dir, int* next_dir, int iter=0) -> bool
	{
		const int back_dir[2] = { -dir[0], -dir[1] };

		if(back_dir[0] == -1 && back_dir[1] == 0)
		{
			switch(iter)
			{
				case 0: next_dir[0] = -1; next_dir[1] = -1; return true;
				case 1: next_dir[0] = 0; next_dir[1] = -1; return true;
				case 2: next_dir[0] = 1; next_dir[1] = -1; return true;
				case 3: next_dir[0] = 1; next_dir[1] = 0; return true;
				case 4: next_dir[0] = 1; next_dir[1] = 1; return true;
				case 5: next_dir[0] = 0; next_dir[1] = 1; return true;
				case 6: next_dir[0] = -1; next_dir[1] = 1; return true;
				default: return false;
			}
		}
		else if(back_dir[0] == -1 && back_dir[1] == -1)
		{
			switch(iter)
			{
				case 0: next_dir[0] = 0; next_dir[1] = -1; return true;
				case 1: next_dir[0] = 1; next_dir[1] = -1; return true;
				case 2: next_dir[0] = 1; next_dir[1] = 0; return true;
				case 3: next_dir[0] = 1; next_dir[1] = 1; return true;
				case 4: next_dir[0] = 0; next_dir[1] = 1; return true;
				case 5: next_dir[0] = -1; next_dir[1] = 1; return true;
				case 6: next_dir[0] = -1; next_dir[1] = 0; return true;
				default: return false;
			}
		}
		else if(back_dir[0] == 0 && back_dir[1] == -1)
		{
			switch(iter)
			{
				case 0: next_dir[0] = 1; next_dir[1] = -1; return true;
				case 1: next_dir[0] = 1; next_dir[1] = 0; return true;
				case 2: next_dir[0] = 1; next_dir[1] = 1; return true;
				case 3: next_dir[0] = 0; next_dir[1] = 1; return true;
				case 4: next_dir[0] = -1; next_dir[1] = 1; return true;
				case 5: next_dir[0] = -1; next_dir[1] = 0; return true;
				case 6: next_dir[0] = -1; next_dir[1] = -1; return true;
				default: return false;
			}
		}
		else if(back_dir[0] == 1 && back_dir[1] == -1)
		{
			switch(iter)
			{
				case 0: next_dir[0] = 1; next_dir[1] = 0; return true;
				case 1: next_dir[0] = 1; next_dir[1] = 1; return true;
				case 2: next_dir[0] = 0; next_dir[1] = 1; return true;
				case 3: next_dir[0] = -1; next_dir[1] = 1; return true;
				case 4: next_dir[0] = -1; next_dir[1] = 0; return true;
				case 5: next_dir[0] = -1; next_dir[1] = -1; return true;
				case 6: next_dir[0] = 0; next_dir[1] = -1; return true;
				default: return false;
			}
		}
		else if(back_dir[0] == 1 && back_dir[1] == 0)
		{
			switch(iter)
			{
				case 0: next_dir[0] = 1; next_dir[1] = 1; return true;
				case 1: next_dir[0] = 0; next_dir[1] = 1; return true;
				case 2: next_dir[0] = -1; next_dir[1] = 1; return true;
				case 3: next_dir[0] = -1; next_dir[1] = 0; return true;
				case 4: next_dir[0] = -1; next_dir[1] = -1; return true;
				case 5: next_dir[0] = 0; next_dir[1] = -1; return true;
				case 6: next_dir[0] = 1; next_dir[1] = -1; return true;
				default: return false;
			}
		}
		else if(back_dir[0] == 1 && back_dir[1] == 1)
		{
			switch(iter)
			{
				case 0: next_dir[0] = 0; next_dir[1] = 1; return true;
				case 1: next_dir[0] = -1; next_dir[1] = 1; return true;
				case 2: next_dir[0] = -1; next_dir[1] = 0; return true;
				case 3: next_dir[0] = -1; next_dir[1] = -1; return true;
				case 4: next_dir[0] = 0; next_dir[1] = -1; return true;
				case 5: next_dir[0] = 1; next_dir[1] = -1; return true;
				case 6: next_dir[0] = 1; next_dir[1] = 0; return true;
				default: return false;
			}
		}
		else if(back_dir[0] == 0 && back_dir[1] == 1)
		{
			switch(iter)
			{
				case 0: next_dir[0] = -1; next_dir[1] = 1; return true;
				case 1: next_dir[0] = -1; next_dir[1] = 0; return true;
				case 2: next_dir[0] = -1; next_dir[1] = -1; return true;
				case 3: next_dir[0] = 0; next_dir[1] = -1; return true;
				case 4: next_dir[0] = 1; next_dir[1] = -1; return true;
				case 5: next_dir[0] = 1; next_dir[1] = 0; return true;
				case 6: next_dir[0] = 1; next_dir[1] = 1; return true;
				default: return false;
			}
		}
		else if(back_dir[0] == -1 && back_dir[1] == 1)
		{
			switch(iter)
			{
				case 0: next_dir[0] = -1; next_dir[1] = 0; return true;
				case 1: next_dir[0] = -1; next_dir[1] = -1; return true;
				case 2: next_dir[0] = 0; next_dir[1] = -1; return true;
				case 3: next_dir[0] = 1; next_dir[1] = -1; return true;
				case 4: next_dir[0] = 1; next_dir[1] = 0; return true;
				case 5: next_dir[0] = 1; next_dir[1] = 1; return true;
				case 6: next_dir[0] = 0; next_dir[1] = 1; return true;
				default: return false;
			}
		}

		return false;
	};

	while(1)
	{
		for(int i=0; i<7; ++i)
		{
			if(get_next_dir(dir, next_dir, i))
			{
				if(img.GetPixel(pos[0]+next_dir[0], pos[1]+next_dir[1]))
				{
					has_next_dir = true;
					break;
				}
			}
		}

		if(has_next_dir)
		{
			dir[0] = next_dir[0];
			dir[1] = next_dir[1];

			pos[0] += dir[0];
			pos[1] += dir[1];

			boundary.SetPixel(pos[0], pos[1], 1);
		}
		else
		{
			break;
		}

		// back at start
		if(pos[0] == start[0] && pos[1] == start[1])
			break;
	}

	return boundary;
}


int main()
{
	bool tstimg[20 * 20] =
	{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	};

	Image<bool> img(20, 20);
	img.SetImage(tstimg);
	std::cout << "Image:\n" << img << std::endl;

	auto boundary = trace_boundary(img);
	std::cout << "Boundary:\n" << boundary << std::endl;

	return 0;
}
