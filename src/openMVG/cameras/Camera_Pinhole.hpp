
// Copyright (c) 2015 Pierre MOULON.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENMVG_CAMERA_PINHOLE_HPP
#define OPENMVG_CAMERA_PINHOLE_HPP

#include "openMVG/numeric/numeric.h"
#include "openMVG/cameras/Camera_Common.hpp"
#include "openMVG/geometry/pose3.hpp"

#include <vector>

namespace openMVG
{
namespace cameras
{

/**
* @brief Define an ideal Pinhole camera intrinsics (store a K 3x3 matrix)
* with intrinsic parameters defining the K calibration matrix
*
* Intrinsic camera matrix is \f$ K = \begin{pmatrix} f & 0 & u_0 \\ 0 & f & v_0 \\ 0 & 0 & 1 \end{pmatrix} \f$
*
* @note This is an ideal Pinhole camera because it doesn't handle skew and distortion
* @note The camera does only handle one focal length (ie: \f$ f_x = f_y = f \f$ )
*/
class Pinhole_Intrinsic : public IntrinsicBase
{
  protected:

    /// Intrinsic matrix : Focal & principal point are embed into the calibration matrix K
    Mat3 _K;

    /// Inverse of intrinsic matrix
    Mat3 _Kinv;

  public:

    /**
    * @brief Constructor
    * @param w Width of the image plane
    * @param h Height of the image plane
    * @param focal_length_pix Focal length (in pixel) of the camera
    * @param ppx Principal point on x-axis
    * @param ppy Principal point on y-axis
    */
    Pinhole_Intrinsic(
      unsigned int w = 0, unsigned int h = 0,
      double focal_length_pix = 0.0,
      double ppx = 0.0, double ppy = 0.0 )
      : IntrinsicBase( w, h )
    {
      _K << focal_length_pix, 0., ppx, 0., focal_length_pix, ppy, 0., 0., 1.;
      _Kinv = _K.inverse();
    }

    /**
    * @brief Destructor
    */
    virtual ~Pinhole_Intrinsic() {}

    /**
    * @brief Get type of the intrinsic
    * @retval PINHOLE_CAMERA
    */
    virtual EINTRINSIC getType() const
    {
      return PINHOLE_CAMERA;
    }

    /**
    * @brief Get the intrinsic matrix
    * @return 3x3 intrinsic matrix
    */
    const Mat3& K() const
    {
      return _K;
    }

    /**
    * @brief Get the inverse of the intrinsic matrix
    * @return Inverse of intrinsic matrix
    */
    const Mat3& Kinv() const
    {
      return _Kinv;
    }


    /**
    * @brief Return the value of the focal in pixels
    * @return Focal of the camera (in pixel)
    */
    inline double focal() const
    {
      return _K( 0, 0 );
    }

    /**
    * @brief Get principal point of the camera
    * @return Principal point of the camera
    */
    inline Vec2 principal_point() const
    {
      return Vec2( _K( 0, 2 ), _K( 1, 2 ) );
    }


    /**
    * @brief Get bearing vector of a point given an image coordinate
    * @return bearing vector
    */
    virtual Vec3 operator () ( const Vec2& p ) const
    {
      Vec3 p3( p( 0 ), p( 1 ), 1.0 );
      return ( _Kinv * p3 ).normalized();
    }

    /**
    * @brief Transform a point from the camera plane to the image plane
    * @param p Camera plane point
    * @return Point on image plane
    */
    virtual Vec2 cam2ima( const Vec2& p ) const
    {
      return focal() * p + principal_point();
    }

    /**
    * @brief Transform a point from the image plane to the camera plane
    * @param p Image plane point
    * @return camera plane point
    */
    virtual Vec2 ima2cam( const Vec2& p ) const
    {
      return ( p -  principal_point() ) / focal();
    }

    /**
    * @brief Does the camera model handle a distortion field?
    * @retval false if intrinsic does not hold distortion
    */
    virtual bool have_disto() const
    {
      return false;
    }

    /**
    * @brief Add the distortion field to a point (that is in normalized camera frame)
    * @param p Point before distortion computation (in normalized camera frame)
    * @return point with distortion
    */
    virtual Vec2 add_disto( const Vec2& p ) const
    {
      return p;
    }

    /**
    * @brief Remove the distortion to a camera point (that is in normalized camera frame)
    * @param p Point with distortion
    * @return Point without distortion
    */
    virtual Vec2 remove_disto( const Vec2& p ) const
    {
      return p;
    }

    /**
    * @brief Normalize a given unit pixel error to the camera plane
    * @param value Error in image plane
    * @return error of passing from the image plane to the camera plane
    */
    virtual double imagePlane_toCameraPlaneError( double value ) const
    {
      return value / focal();
    }

    /**
    * @brief Return the projection matrix (interior & exterior) as a simplified projective projection
    * @param pose Extrinsic matrix
    * @return Concatenation of intrinsic matrix and extrinsic matrix
    */
    virtual Mat34 get_projective_equivalent( const geometry::Pose3 & pose ) const
    {
      Mat34 P;
      P_From_KRt( K(), pose.rotation(), pose.translation(), &P );
      return P;
    }


    /**
    * @brief Data wrapper for non linear optimization (get data)
    * @return vector of parameter of this intrinsic
    */
    virtual std::vector<double> getParams() const
    {
      const std::vector<double> params = {_K( 0, 0 ), _K( 0, 2 ), _K( 1, 2 )};
      return params;
    }


    /**
    * @brief Data wrapper for non linear optimization (update from data)
    * @param params List of params used to update this intrinsic
    * @retval true if update is correct
    * @retval false if there was an error during update
    */
    virtual bool updateFromParams( const std::vector<double> & params )
    {
      if ( params.size() == 3 )
      {
        *this = Pinhole_Intrinsic( _w, _h, params[0], params[1], params[2] );
        return true;
      }
      else
      {
        return false;
      }
    }

    /**
    * @brief Return the un-distorted pixel (with removed distortion)
    * @param p Input distorted pixel
    * @return Point without distortion
    */
    virtual Vec2 get_ud_pixel( const Vec2& p ) const
    {
      return p;
    }

    /**
    * @brief Return the distorted pixel (with added distortion)
    * @param p Input pixel
    * @return Distorted pixel
    */
    virtual Vec2 get_d_pixel( const Vec2& p ) const
    {
      return p;
    }

    /**
    * @brief Serialization out
    * @param ar Archive
    */
    template <class Archive>
    void save( Archive & ar ) const
    {
      IntrinsicBase::save( ar );
      ar( cereal::make_nvp( "focal_length", _K( 0, 0 ) ) );
      const std::vector<double> pp = {_K( 0, 2 ), _K( 1, 2 )};
      ar( cereal::make_nvp( "principal_point", pp ) );
    }


    /**
    * @brief  Serialization in
    * @param ar Archive
    */
    template <class Archive>
    void load( Archive & ar )
    {
      IntrinsicBase::load( ar );
      double focal_length;
      ar( cereal::make_nvp( "focal_length", focal_length ) );
      std::vector<double> pp( 2 );
      ar( cereal::make_nvp( "principal_point", pp ) );
      *this = Pinhole_Intrinsic( _w, _h, focal_length, pp[0], pp[1] );
    }
};

} // namespace cameras
} // namespace openMVG

#include <cereal/types/polymorphic.hpp>
#include <cereal/types/vector.hpp>

CEREAL_REGISTER_TYPE_WITH_NAME( openMVG::cameras::Pinhole_Intrinsic, "pinhole" );

#endif // #ifndef OPENMVG_CAMERA_PINHOLE_HPP

