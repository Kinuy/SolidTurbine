#ifndef WVP_ANGLES_H_
#define WVP_ANGLES_H_

#define _USE_MATH_DEFINES
#include <math.h>

// An angle type that keeps the range between
// 0 and 2*pi as standard
// But you can specify upper and lower limits in the constructor also.
// Hopefully it works ok....
class AngleMax2Pi
{
public:
    AngleMax2Pi() {};
    AngleMax2Pi(double init_rad)
        : value_as_radians_(0),
          lower_lim_(0),
          upper_lim_(2 * M_PI)
    {
        value_as_radians_ = mod_val(init_rad);
    };
    AngleMax2Pi(double init_rad, double lower_lim, double upper_lim)
        : value_as_radians_(0),
          lower_lim_(lower_lim),
          upper_lim_(upper_lim)
    {
        value_as_radians_ = mod_val(init_rad);
    };
    double as_radians() const
    {
        return value_as_radians_;
    };
    double as_degrees() const
    {
        return value_as_radians_ * 180 / M_PI;
    };
    void set_with_radians(double x)
    {
        value_as_radians_ = mod_val(x);
    };
    void set_with_degrees(double x)
    {
        value_as_radians_ = mod_val(M_PI / 180 * x);
    };
    double mod_val(double val) const
    {
        while (val < lower_lim_)
        {
            val += 2 * M_PI;
        }
        while (val >= upper_lim_)
        {
            val -= 2 * M_PI;
        }
        return val;
    };
    friend AngleMax2Pi operator+(AngleMax2Pi const &l, AngleMax2Pi const &r);
    friend AngleMax2Pi operator-(AngleMax2Pi const &l, AngleMax2Pi const &r);
    AngleMax2Pi &operator+=(AngleMax2Pi const &r)
    {
        value_as_radians_ = mod_val(value_as_radians_ + r.value_as_radians_);
        return *this;
    };
    AngleMax2Pi &operator-=(AngleMax2Pi const &r)
    {
        value_as_radians_ = mod_val(value_as_radians_ - r.value_as_radians_);
        return *this;
    };
    AngleMax2Pi &operator+(double const &x)
    {
        value_as_radians_ = mod_val(value_as_radians_ + x);
        return *this;
    };
    AngleMax2Pi &operator-(double const &x)
    {
        value_as_radians_ = mod_val(value_as_radians_ - x);
        return *this;
    };

private:
    double value_as_radians_;
    double lower_lim_;
    double upper_lim_;
};

inline AngleMax2Pi operator+(AngleMax2Pi const &l, AngleMax2Pi const &r)
{
    return AngleMax2Pi(l.as_radians() + r.as_radians());
};
inline AngleMax2Pi operator-(AngleMax2Pi const &l, AngleMax2Pi const &r)
{
    return AngleMax2Pi(l.as_radians() - r.as_radians());
};
#endif // WVP_ANGLES_H_
