/*
 * @file  CSVNetwork.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#include "CSVNetwork.h"

#include <QFile>
#include <QStringList>


namespace simil
{

  CSVNetwork::CSVNetwork( const std::string& filename,
                          char separator,
                          bool headerLine )
  : _fileName( filename )
  , _separator( separator )
  , _headerLine( headerLine )
  { }

  CSVNetwork::~CSVNetwork( )
  { }

  void CSVNetwork::load( void )
  {
    QFile file( QString( _fileName.data( )));
    if( !file.open( QIODevice::ReadOnly | QFile::Text))
    {
      std::cerr << "Error: Could not open CSV file " << _fileName
                << "." << std::endl;
      return;
    }

//    if( _headerLine )
//      file.readLine( );

    bool ok = false;
    bool includesGID = false;
    unsigned int counter = 0;
    unsigned int gid = 0;

    while( !file.atEnd( ))
    {
      QStringList stringLine;
      QByteArray line = file.readLine( );
      QList< QByteArray > wordList = line.split( _separator );

      if( wordList.size( ) < 3 || wordList.size( ) > 4 )
      {
        std::cerr << std::endl
                  << "CSV error in line: " << counter
                  << ". Please check file format and make sure all lines match the following structure for each line:"
                  << " '[GID,]X,Y,Z'"
                  << " where the GID is an optional field, and  X,Y,Z are the 3D coordinates."
                  << std::endl;
        exit( -1 );
      }

      for( auto word : wordList )
        stringLine.append( word.constData( ));

      vmml::Vector3f coordinates;

      includesGID = wordList.size( ) > 3;

      gid = ( includesGID ) ? stringLine[ 0 ].toUInt( &ok ) : counter;

      unsigned int i = 0;
      for( auto word : stringLine )
      {

        if( includesGID && i == 0 )
        {
          ++i;
          continue;
        }

        float value = word.toFloat( &ok );

        if( !ok )
          std::cout << "Warning: Value " << word.toStdString( ) << " not converted to float." << std::endl;

        coordinates[ i ] = value;

        i++;
      }

      _gids.insert( gid );
      _positions.push_back( coordinates );

      counter++;
    }

    file.close( );
  }

  void CSVNetwork::clear( void )
  {

  }

  simil::TGIDSet CSVNetwork::getGIDs( void ) const
  {
    return _gids;
  }

  simil::TPosVect CSVNetwork::getComposedPositions( void ) const
  {
    return _positions;
  }


}
