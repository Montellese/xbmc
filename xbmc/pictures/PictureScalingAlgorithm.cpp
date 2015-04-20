/*
 *      Copyright (C) 2015 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

extern "C" {
#include "libswscale/swscale.h"
}

#include "PictureScalingAlgorithm.h"
#include "utils/StringUtils.h"

CPictureScalingAlgorithm::Algorithm CPictureScalingAlgorithm::Default = CPictureScalingAlgorithm::FastBilinear;

CPictureScalingAlgorithm::Algorithm CPictureScalingAlgorithm::FromString(const std::string& scalingAlgorithm)
{
  if (StringUtils::EqualsNoCase(scalingAlgorithm, "fast") || StringUtils::EqualsNoCase(scalingAlgorithm, "fast_bilinear"))
    return FastBilinear;
  if (StringUtils::EqualsNoCase(scalingAlgorithm, "bilinear"))
    return Bilinear;
  if (StringUtils::EqualsNoCase(scalingAlgorithm, "bicubic"))
    return Bicubic;
  if (StringUtils::EqualsNoCase(scalingAlgorithm, "experimental"))
    return Experimental;
  if (StringUtils::EqualsNoCase(scalingAlgorithm, "neighbor") || StringUtils::EqualsNoCase(scalingAlgorithm, "nearest_neighbor"))
    return NearestNeighbor;
  if (StringUtils::EqualsNoCase(scalingAlgorithm, "area") || StringUtils::EqualsNoCase(scalingAlgorithm, "averaging_neighbor"))
    return AveragingArea;
  if (StringUtils::EqualsNoCase(scalingAlgorithm, "bicublin"))
    return Bicublin;
  if (StringUtils::EqualsNoCase(scalingAlgorithm, "gaussian"))
    return Gaussian;
  if (StringUtils::EqualsNoCase(scalingAlgorithm, "sinc"))
    return Sinc;
  if (StringUtils::EqualsNoCase(scalingAlgorithm, "lanczos"))
    return Lanczos;
  if (StringUtils::EqualsNoCase(scalingAlgorithm, "spline") || StringUtils::EqualsNoCase(scalingAlgorithm, "bicubic_neighbor"))
    return BicubicSpline;

  return None;
}

std::string CPictureScalingAlgorithm::ToString(Algorithm scalingAlgorithm)
{
  switch (scalingAlgorithm)
  {
  case FastBilinear:
    return "fast_bilinear";

  case Bilinear:
    return "bilinear";

  case Bicubic:
    return "bicubic";

  case Experimental:
    return "experimental";

  case NearestNeighbor:
    return "neighbor";

  case AveragingArea:
    return "area";

  case Bicublin:
    return "bicublin";

  case Gaussian:
    return "gaussian";

  case Sinc:
    return "sinc";

  case Lanczos:
    return "lanczos";

  case BicubicSpline:
    return "spline";

  default:
    break;
  }

  return "";
}

int CPictureScalingAlgorithm::ToSwscale(const std::string& scalingAlgorithm)
{
  return ToSwscale(FromString(scalingAlgorithm));
}

int CPictureScalingAlgorithm::ToSwscale(Algorithm scalingAlgorithm)
{
  switch (scalingAlgorithm)
  {
  case FastBilinear:
    return SWS_FAST_BILINEAR;

  case Bilinear:
    return SWS_BILINEAR;

  case Bicubic:
    return SWS_BICUBIC;

  case Experimental:
    return SWS_X;

  case NearestNeighbor:
    return SWS_POINT;

  case AveragingArea:
    return SWS_AREA;

  case Bicublin:
    return SWS_BICUBLIN;

  case Gaussian:
    return SWS_GAUSS;

  case Sinc:
    return SWS_SINC;

  case Lanczos:
    return SWS_LANCZOS;

  case BicubicSpline:
    return SWS_SPLINE;

  default:
    break;
  }

  return ToSwscale(Default);
}
