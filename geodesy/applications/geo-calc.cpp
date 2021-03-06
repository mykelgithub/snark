// This file is part of snark, a generic and flexible library for robotics research
// Copyright (c) 2011 The University of Sydney
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. Neither the name of the University of Sydney nor the
//    names of its contributors may be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
// GRANTED BY THIS LICENSE.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
// HOLDERS AND CONTRIBUTORS \"AS IS\" AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
// IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <functional>
#include <comma/application/command_line_options.h>
#include <comma/csv/stream.h>
#include "../../math/spherical_geometry/traits.h"
#include <boost/graph/graph_concepts.hpp>
#include "../geoids.h"

static void usage( bool more = false )
{
    std::cerr << std::endl;
    std::cerr << "take coordinates in degrees on geoid from stdin, perform calculations, append result and output to stdout" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage examples" << std::endl;
    std::cerr << "    geo-calc <operation> [<options>]" << std::endl;
    std::cerr << "    cat arcs.csv | geo-calc distance [<options>] > results.csv" << std::endl;
    std::cerr << "    cat circular-arcs.csv | geo-calc discretize arc [<options>] > results.csv" << std::endl;
    std::cerr << "    cat circle.csv | geo-calc discretize circle [<options>]  > results.csv" << std::endl;
    std::cerr << std::endl;
    std::cerr << "operations" << std::endl;
    std::cerr << "    distance   : output length of ellipsoid arc" << std::endl;
    std::cerr << "    discretize : output a discretized shape (e.g. circle) with a given resolution" << std::endl;
    std::cerr << std::endl;
    std::cerr << "operations: details" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    distance: ellipsoid arc distance in meters; if --binary, output as double" << std::endl;
    std::cerr << std::endl;
    std::cerr << "        input fields: " << comma::join( comma::csv::names< snark::spherical::ellipsoid::arc >( true ), ',' ) << std::endl;
    std::cerr << std::endl;
    std::cerr << "    discretize <shape>: output a discretized shape with a given resolution" << std::endl;
    std::cerr << "                multiple output lines per input line" << std::endl;
    std::cerr << "                the result will be appended to the line (see example)" << std::endl;
    std::cerr << "          shape: circle, arc" << std::endl;
    std::cerr << "          input fields" << std::endl;
    std::cerr << "              circle: centre,radius" << std::endl;
    std::cerr << "                  centre: latitude,longitude in degrees" << std::endl;
    std::cerr << "                  radius: metric distance from centre" << std::endl;
    std::cerr << "              arc: circle/centre,circle/radius,begin,end" << std::endl;
    std::cerr << "                  circle/centre: latitude,longitude in degrees" << std::endl;
    std::cerr << "                  circle/radius: metric distance from centre" << std::endl;
    std::cerr << "                  begin: begin bearing, degrees" << std::endl;
    std::cerr << "                  end: end bearing, degrees" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    info: get geoid's information; output format is name,description,major_semiaxis,minor_semiaxis,eccentricity" << std::endl;
    std::cerr << std::endl;
    std::cerr << "        options" << std::endl;
    std::cerr << "            --resolution=<degrees>" << std::endl;
    std::cerr << "            --circle-size,--size=<number>: number of points in the discretized circle (circle or arc only)" << std::endl;
    std::cerr << "                                           if both --resolution and --cricle-size present, whatever gives finer resolution will be choosen" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << "    --help,-h: show help; --help --verbose for more help" << std::endl;
    std::cerr << "    --verbose,-v: more info" << std::endl;
    std::cerr << "    --geoid: <geoid name> (case insensitive); default: WGS84 " << std::endl;
    std::cerr << "        geoids:" << std::endl;
    std::cerr << "            WGS84 (default): standard used in GPS" << std::endl;
    std::cerr << "            AGD84: Australian standard" << std::endl;
    std::cerr << "            GRS67: Historical standard" << std::endl;
    std::cerr << "            sphere: major=minor" << std::endl;
    std::cerr << "            unit: scaled down WGS84 (~radian)" << std::endl;

    if ( more )
    {
        std::cerr << "        (name: description (major semiaxis; minor semiaxis); inverse eccentricity)" << std::endl;
        snark::geodesy::wgs84::help();
        snark::geodesy::agd84::help();
        snark::geodesy::grs67::help();
        snark::geodesy::geoids::other_help();
    }
    else
    {
        std::cerr << "           run geo-calc --help --verbose for more details..." << std::endl;
    }
    if ( more )
        std::cerr << std::endl << "csv options" << std::endl << comma::csv::options::usage() << std::endl;
    std::cerr << std::endl;
    std::cerr << "examples" << std::endl;
    std::cerr << "    cat circular-arcs.csv | geo-calc discretize arc --fields=circle/centre/latitude,circle/centre/longitude,circle/radius,begin,end --circle-size=32 > results.csv" << std::endl;
    std::cerr << "    cat circle.csv | geo-calc discretize circle --fields=centre,radius --resolution=0.1 | column -ts," << std::endl;
    std::cerr << std::endl;
    exit( 1 );
}

template < typename S >
int discretize( const comma::csv::options &csv, snark::spherical::ellipsoid &ellipsoid, const boost::optional< double > &resolution, const boost::optional< unsigned int > &circle_size )
{
    comma::csv::input_stream< S > istream( std::cin, csv );
    comma::csv::ascii< snark::spherical::coordinates > ascii;
    comma::csv::binary< snark::spherical::coordinates > binary;
    while ( istream.ready() || std::cin.good() )
    {
        const S *a = istream.read();
        if ( !a ) { break; }
        const std::vector< snark::spherical::coordinates > &v = a->discretize( ellipsoid, resolution, circle_size );
        for ( unsigned int i = 0; i < v.size(); ++i )
        {
            if ( csv.binary() )
            {
                std::cout.write( istream.binary().last(), istream.binary().binary().format().size() );
                std::vector< char > buf( binary.format().size() );
                binary.put( v[i], &buf[0] );
                std::cout.write( reinterpret_cast< const char * >( &buf[i] ), buf.size() );
            }
            else
            {
                std::cout << comma::join( istream.ascii().last(), csv.delimiter );
                std::cout << csv.delimiter << ascii.put( v[i] );
                std::cout << std::endl;
            }
        }
    }
    return 0;
}

int main( int ac, char **av )
{
    try
    {
        snark::geodesy::geoids geoids;
        comma::command_line_options options( ac, av );
        bool verbose = options.exists( "--verbose,-v" );
        if ( options.exists( "--help,-h" ) )
            usage( verbose );
        comma::csv::options csv( options );
        if ( !csv.binary() )
            std::cout.precision( options.value( "--precision,-p", 12 ) );
        csv.full_xpath = true;
        snark::spherical::ellipsoid *geoid = geoids.select( options.optional< std::string >( "--geoid" ) );
        if ( geoid == NULL )
        {
            std::cerr << "geoid not supported: " << geoids.name << std::endl;
            return 1;
        }
        const std::vector<std::string> &operations = options.unnamed( "--verbose,-v,--degrees", "-.*" );
        if ( operations.size() < 1 )
        {
            std::cerr << "geo-calc: expected one operation, got " << operations.size() << std::endl;
            return 1;
        }
        if ( operations[0] == "distance" )
        {
            comma::csv::input_stream< snark::spherical::ellipsoid::arc > istream( std::cin, csv );
            while ( istream.ready() || ( std::cin.good() && !std::cin.eof() ) )
            {
                const snark::spherical::ellipsoid::arc *a = istream.read();
                if ( !a )
                    break;
                double distance = geoid->distance( a->begin, a->end );
                if ( csv.binary() )
                {
                    std::cout.write( istream.binary().last(), istream.binary().binary().format().size() );
                    std::cout.write( reinterpret_cast< const char * >( &distance ), sizeof( double ) );
                }
                else
                    std::cout << comma::join( istream.ascii().last(), csv.delimiter ) << csv.delimiter << distance << std::endl;
            }
            return 0;
        }
        else if ( operations[0] == "discretize" )
        {
            //whole circle or circular arc
            boost::optional< double > resolution = options.optional< double >( "--resolution" );
            if ( resolution ) { resolution = *resolution * M_PI / 180; }
            boost::optional< unsigned int > circle_size = options.optional< unsigned int >( "--circle-size,--size" );
            if ( operations.size() == 2 )
            {
                if ( operations[1] == "circle" )
                {
                    return discretize< snark::spherical::ellipsoid::circle >( csv, *geoid, resolution, circle_size );
                }
                else if ( operations[1] == "arc" )
                {
                    return discretize<snark::spherical::ellipsoid::circle::arc>( csv, *geoid, resolution, circle_size );
                }
                std::cerr << "geo-calc: unknown shape for discretize: \"" << operations[1] << "\"" << std::endl;
            }
            else
                std::cerr << "geo-calc: expected shape for operation discretize" << std::endl;
            return 1;
        }
        else if ( operations[0] == "info" )
        {
            if ( geoids.info != NULL )
            {
                geoids.info();
                return 0;
            }
            std::cerr << "can't get info on geoid" << std::endl;
            return 1;

        }
        std::cerr << "geo-calc: unknown operation: \"" << operations[0] << "\"" << std::endl;
        return 1;
    }
    catch ( std::exception &ex )
    {
        std::cerr << "geo-calc: " << ex.what() << std::endl;
    }
    catch ( ... )
    {
        std::cerr << "geo-calc: unknown exception" << std::endl;
    }
    return 1;
}
