/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2013 CERN
 * @author Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef __SEG_H
#define __SEG_H

#include <cstdio>
#include <climits>

#include <math/vector2d.h>

#include <boost/optional/optional.hpp>

typedef boost::optional<VECTOR2I> OPT_VECTOR2I;

class SEG {

	private:
		typedef VECTOR2I::extended_type ecoord;

	public:

		friend inline std::ostream& operator<<( std::ostream& aStream, const SEG& aSeg );

		/* Start and the of the segment. Public, to make access simpler. These are references
		 * to an object the segment belongs to (e.g. a line chain) or references to locally stored points
		 * (m_a, m_b).
		 */
		VECTOR2I& a, b;

		/** Default constructor
		 * Creates an empty (0, 0) segment, locally-referenced 
		 */
		SEG(): a(m_a), b(m_b)
		{
			a = m_a;
			b = m_b;
			m_is_local = true;
			m_index = -1;
		}

		/** 
		 * Constructor
		 * Creates a segment between (x1, y1) and (x2, y2), locally referenced
		 */
		SEG ( int x1, int y1, int x2, int y2 ) : a(m_a), b(m_b)
		{
			m_a = VECTOR2I(x1, y1);
			m_b = VECTOR2I(x2, y2);
			a = m_a;
			b = m_b;
			m_is_local = true;
			m_index = -1;
		}

		/** 
		 * Constructor
		 * Creates a segment between (aA) and (aB), locally referenced
		 */
		SEG ( const VECTOR2I& aA, const VECTOR2I& aB ): a(m_a), b(m_b), m_a(aA), m_b(aB)
		{
			a = m_a;
			b = m_b;
			m_is_local = true;
			m_index = -1;
		}
	
		/**
		 * Constructor
		 * Creates a segment between (aA) and (aB), referenced to a multi-segment shape
		 * @param aA reference to the start point in the parent shape
		 * @param aB reference to the end point in the parent shape
		 * @param aIndex index of the segment within the parent shape
		 */
		SEG ( VECTOR2I& aA, VECTOR2I& aB, int aIndex ): a(aA), b(aB)
		{
			m_is_local = false;
			m_index = aIndex;
		}

		/**
		 * Copy constructor
		 */
		SEG ( const SEG& seg ): a(m_a), b(m_b)
		{
			if (seg.m_is_local)
			{
				m_a = seg.m_a;
				m_b = seg.m_b;
				a = m_a;
				b = m_b;
				m_is_local = true;
				m_index = -1;
			} else {
				a = seg.a;
				b = seg.b;
				m_index = seg.m_index;
				m_is_local = false;
			}
		}

		SEG& operator=(const SEG& seg)
		{
			a = seg.a;
			b = seg.b;
			m_a = seg.m_a;
			m_b = seg.m_b;
			m_index = seg.m_index;
			m_is_local = seg.m_is_local;
			return *this;
		}

    	/**
     	 * Function LineProject()
     	 *
     	 * Computes the perpendicular projection point of aP on a line passing through
     	 * ends of the segment.
     	 * @param aP point to project
     	 * @return projected point
     	 */
    	VECTOR2I LineProject( const VECTOR2I& aP ) const;

    	/**
     	 * Function Side()
     	 *
     	 * Determines on which side of directed line passing via segment ends point aP lies.
     	 * @param aP point to determine the orientation wrs to self
     	 * @return: < 0: left, 0 : on the line, > 0 : right
     	 */
    	int Side( const VECTOR2I& aP ) const
    	{
		    const ecoord det = (b - a).Cross(aP - a); 
    	 	return det < 0 ? -1 : (det > 0 ? 1 : 0);
		}

    	/**
     	 * Function LineDistance()
     	 *
     	 * Returns the closest Euclidean distance between point aP and the line defined by 
     	 * the ends of segment (this).
     	 * @param aDetermineSide: when true, the sign of the returned value indicates
     	 * the side of the line at which we are (negative = left)
     	 * @return the distance
     	 */
    	int LineDistance( const VECTOR2I& aP, bool aDetermineSide = false ) const;

    	/**
     	 * Function NearestPoint()
     	 *
     	 * Computes a point on the segment (this) that is closest to point aP.
     	 * @return: nearest point
     	 */
    	const VECTOR2I NearestPoint( const VECTOR2I &aP ) const;


    	/**
    	 * Function Intersect()
    	 *
    	 * Computes intersection point of segment (this) with segment aSeg.
    	 * @param aSeg: segment to intersect with
    	 * @param aIgnoreEndpoints: don't treat corner cases (i.e. end of one segment touching the other)
    	 * as intersections.
    	 * @param aLines: treat segments as infinite lines
    	 * @return intersection point, if exists
    	 */
		OPT_VECTOR2I Intersect( const SEG& aSeg, bool aIgnoreEndpoints = false, bool aLines = false ) const;



		/**
		 * Function IntersectLines()
		 *
		 * Computes the intersection point of lines passing through ends of (this) and aSeg
		 * @param aSeg segment defining the line to intersect with
		 * @return intersection point, if exists
		 */
		OPT_VECTOR2I IntersectLines(  const SEG& aSeg ) const
		{
			return Intersect ( aSeg, false, true );
		}

		bool Collide( const SEG& aSeg, int aClearance ) const;

		/**
		 * Function Distance()
		 * 
		 * Computes minimum Euclidean distance to segment aSeg.
		 * @param aSeg other segment
		 * @return minimum distance
		 */
		
		ecoord SquaredDistance( const SEG& aSeg ) const ;
		
		int Distance( const SEG& aSeg ) const 
		{
			return sqrt ( SquaredDistance(aSeg) );
		}
		

		/**
		 * Function Distance()
		 * 
		 * Computes minimum Euclidean distance to point aP.
		 * @param aP the point
		 * @return minimum distance
		 */
		 
		ecoord SquaredDistance( const VECTOR2I& aP ) const 
		{
			return (NearestPoint(aP) - aP).SquaredEuclideanNorm();
		}

		int Distance( const VECTOR2I& aP ) const 
		{
			return sqrt ( SquaredDistance( aP) );
		}

		
		/**
		 * Function Collinear()
		 * 
		 * Checks if segment aSeg lies on the same line as (this).
		 * @param aSeg the segment to chech colinearity with
		 * @return true, when segments are collinear.
		 */

		bool Collinear( const SEG& aSeg ) const 
		{
			ecoord qa1 = a.y - b.y;
			ecoord qb1 = b.x - a.x;
			ecoord qc1 = -qa1 * a.x - qb1 * a.y;
			ecoord qa2 = aSeg.a.y - aSeg.b.y;
			ecoord qb2 = aSeg.b.x - aSeg.a.x;
			ecoord qc2 = -qa2 * aSeg.a.x - qb2 * aSeg.a.y;

			return (qa1 == qa2) && (qb1 == qb2) && (qc1 == qc2);
		}

		/**
		 * Function Length()
		 * 
		 * Returns the length (this)
		 * @return length
		 */
		int Length() const
		{
			return (a - b).EuclideanNorm();
		}

		/**
		 * Function Index()
		 * 
		 * Return the index of this segment in its parent shape (applicable only to non-local segments)
		 * @return index value
		 */
		int Index() const
		{
			return m_index;
		}


		bool Contains(const VECTOR2I& aP) const;

		bool PointCloserThan ( const VECTOR2I& aP, int dist) const;

	//	friend std::ostream& operator<<( std::ostream& stream, const SEG& aSeg );
	private:

		bool ccw ( const VECTOR2I& a, const VECTOR2I& b, const VECTOR2I &c ) const;
		
		///> locally stored start/end coordinates (used when m_is_local == true)
		VECTOR2I m_a, m_b;
		///> index withing the parent shape (used when m_is_local == false)
		int m_index;
		///> locality flag
		bool m_is_local;
};

inline VECTOR2I SEG::LineProject( const VECTOR2I& aP ) const
{
	// fixme: numerical errors for large integers
	assert(false);
    /*const VECTOR2I   		  d 	  = aB - aA;
    ecoord   det     = d.Dot(d);
    ecoord   dxdy    = (ecoord) d.x * d.y;
    
    ecoord   qx      =
        ( (extended_type) aA.x * d.y * d.y + (extended_type) d.x * d.x * x - dxdy *
          (aA.y - y) ) / det;
    extended_type   qy =
        ( (extended_type) aA.y * d.x * d.x + (extended_type) d.y * d.y * y - dxdy *
          (aA.x - x) ) / det;

    return VECTOR2<T> ( (T) qx, (T) qy );*/
}



inline int SEG::LineDistance( const VECTOR2I& aP, bool aDetermineSide ) const
{
    ecoord   p   = a.y - b.y;
    ecoord   q   = b.x - a.x;
	ecoord   r   = -p * a.x - q * a.y;

    ecoord   dist = ( p * aP.x + q * aP.y + r ) / sqrt( p * p + q * q );
    return aDetermineSide ? dist : abs(dist);
}



inline const VECTOR2I SEG::NearestPoint(const VECTOR2I& aP) const
{
    VECTOR2I   		  d = b - a;
    ecoord   l_squared = d.Dot(d);

    if( l_squared == 0 )
        return a;
    
    ecoord t = d.Dot(aP - a);
        
    if( t < 0 )
        return a;
    else if( t > l_squared )
        return b;
    
    int xp = rescale(t, (ecoord)d.x, l_squared);
    int yp = rescale(t, (ecoord)d.y, l_squared);

    return a + VECTOR2I(xp, yp);
}

inline std::ostream& operator<<( std::ostream& aStream, const SEG& aSeg )
{
    if(aSeg.m_is_local)
    	aStream << "[ local " << aSeg.a << " - " << aSeg.b << " ]";
    return aStream;
}

#endif // __SEG_H
