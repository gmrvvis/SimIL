/*
 * @file  LoadInsituData.h
 * @brief
 * @author Aaron Sújar <aaron.sujar@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#include "LoadRestApiData.h"
#include "../SimulationData.h"

#include "HTTPSyncClient.h"

#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace simil
{
  // Cone _cone;
  LoadRestApiData::LoadRestApiData( )
    : LoadSimData( )
    //, _dataset( nullptr )
    , _instance( nullptr )
    , _simulationdata( nullptr )
    , _waitForData( false )
    , _host( "localhost" )
    , _port( 8000 )
  {
  }

  LoadRestApiData::~LoadRestApiData( )
  {
    _waitForData = false;
    _spikeslooper.join( );
    _networklooper.join( );
  }

  SimulationData* LoadRestApiData::LoadSimulationData( const std::string&,
                                                       const std::string& )
  {
    if ( _simulationdata == nullptr )
      _simulationdata = new SpikeData( );

    _waitForData = true;
    _networklooper = std::thread( &LoadRestApiData::Networkloop, this );
    _spikeslooper = std::thread( &LoadRestApiData::Spikeloop, this );

    return _simulationdata;
  } // namespace simil

  void LoadRestApiData::SpikeCB( std::istream& contentdata )
  {
    boost::property_tree::ptree propertytree;
    try {
     boost::property_tree::read_json( contentdata, propertytree );
    } catch (std::exception& e)
    {
      std::cerr << "Exception JSON PARSER:  " << e.what() << "\n";
      return;
    }

    // neuron_ids
    // simulation_steps/

    SpikeData* _spikes = dynamic_cast< SpikeData* >( _simulationdata );

    float startTime = _spikes->startTime( );
    float endTime = _spikes->endTime( );

    auto it_gids = propertytree.get_child( "neuron_ids" ).begin( );
    auto it_times = propertytree.get_child( "simulation_steps" ).begin( );
    auto gids_end = propertytree.get_child( "neuron_ids" ).end( );

    for ( ; it_gids != gids_end; ++it_gids, ++it_times )
    {
      float timestamp = it_times->second.get_value< unsigned int >( ) * 0.1f;
      if ( timestamp < startTime )
        _spikes->setStartTime( timestamp );
      if ( timestamp > endTime )
      {
        _spikes->setEndTime( timestamp );
        _spikes->addSpike( timestamp,
                           it_gids->second.get_value< unsigned int >( ) );
      }
    }
  }

  void LoadRestApiData::GidsCB( std::istream& contentdata )
  {
    boost::property_tree::ptree propertytree;
    boost::property_tree::read_json( contentdata, propertytree );
  }

  void LoadRestApiData::PopulationsCB( std::istream& contentdata )
  {
    boost::property_tree::ptree propertytree;
    boost::property_tree::read_json( contentdata, propertytree );
  }

  void LoadRestApiData::NPropertiesCB( std::istream& contentdata )
  {
    boost::property_tree::ptree propertytree;
    boost::property_tree::read_json( contentdata, propertytree );
  }

  void LoadRestApiData::TimeCB( std::istream& contentdata )
  {
    boost::property_tree::ptree propertytree;
    boost::property_tree::read_json( contentdata, propertytree );
  }

  /*void LoadRestApiData::handlerStatic( const HTTPRequest& request,
                                       HTTPResponse& response,
                                       boost::system::error_code error_code )
  {
    if ( error_code == 0 )
    {
      if ( response.get_status_code( ) == 200 )
      {
        std::cout << "Request #" << request.get_id( )
                  << " has completed. Response: ";
        //<< response.get_response().rdbuf();
        LoadRestApiData* loader =
          static_cast< LoadRestApiData* >( request.get_userpointer( ) );

        switch ( request.get_handlerId( ) )
        {
          case GETRequest::Spikes:
            loader->SpikeCB( response.get_response( ) );
            break;
          case GETRequest::Gids:
            loader->GidsCB( response.get_response( ) );
            break;
          case GETRequest::NeuronPro:
            loader->NPropertiesCB( response.get_response( ) );
            break;
          case GETRequest::TimeInfo:
            loader->TimeCB( response.get_response( ) );
            break;
          case GETRequest::Populations:
            loader->PopulationsCB( response.get_response( ) );
            break;
          default:
            break;
        }
      }
      else
        std::cerr << "Status code:" << response.get_status_message( ) << ":"
                  << response.get_status_code( ) << "\n";
    }
    else if ( error_code == boost::asio::error::operation_aborted )
    {
      std::cerr << "Request #" << request.get_id( )
                << " has been cancelled by the user." << std::endl;
    }
    else
    {
      std::cerr << "Request #" << request.get_id( )
                << " failed! Error code = " << error_code.value( )
                << ". Error message = " << error_code.message( ) << std::endl;
    }
  }*/

  void LoadRestApiData::Spikeloop( )
  {
    while ( _waitForData )
    {
      if ( GETSpikes( ) == 0 )
        std::this_thread::sleep_for( std::chrono::milliseconds( 300 ) );
      else
        std::this_thread::sleep_for( std::chrono::milliseconds( 300 ) );
    }
  }
  void LoadRestApiData::Networkloop( )
  {
    while ( _waitForData )
    {
      /*auto gids = NetData.GetNeuronIds( );
      auto numGids = gids.number_of_elements( );
      for ( unsigned int i = 0; i < numGids; i++ )
      {
        _simulationdata->setGid( gids[ i ] );
      }

      auto pos = NetData.GetFloatingPointAttributeValues( "Positions" );
      auto numPos = pos.number_of_elements( );
      for ( unsigned int i = 0; i < numPos; i++ )
      {
        _simulationdata->setPosition(
          vmml::Vector3f( pos[ i ], pos[ i ], pos[ i ] ) );
      }*/

      GETTimeInfo( );
      GETGids( );
      GETPopulations( );

      std::this_thread::sleep_for( std::chrono::seconds( 3 ) );

    }
  }

  int LoadRestApiData::GETTimeInfo( )
  {
    HTTPSyncClient client;

    client.set_host( _host );
    client.set_uri( "/simulation_time_info" );
    client.set_port( _port );

    int result = client.execute( );

    if ( result == 0 ) // Success
    {
      TimeCB( client.get_response( ) );
    }

    return result;
  }
  int LoadRestApiData::GETGids( )
  {
    HTTPSyncClient client;

    client.set_host( _host );
    client.set_uri( "/gids" );
    client.set_port( _port );

    int result = client.execute( );

    if ( result == 0 ) // Success
    {
      GidsCB( client.get_response( ) );
    }
    return result;
  }
  int LoadRestApiData::GETNeuronProperties( )
  {
    HTTPSyncClient client;

    client.set_host( _host );
    client.set_uri( "/neuron_properties" );
    client.set_port( _port );

    int result = client.execute( );

    if ( result == 0 ) // Success
    {
      NPropertiesCB( client.get_response( ) );
    }
    return result;
  }
  int LoadRestApiData::GETPopulations( )
  {
    HTTPSyncClient client;

    client.set_host( _host );
    client.set_uri( "/populations" );
    client.set_port( _port );

    int result = client.execute( );

    if ( result == 0 ) // Success
    {
      PopulationsCB( client.get_response( ) );
    }

    return result;
  }
  int LoadRestApiData::GETSpikes( )
  {
    HTTPSyncClient client;

    client.set_host( _host );
    client.set_uri( "/spikes" );
    client.set_port( _port );

    int result = client.execute( );

    if ( result == 0 ) // Success
    {
      SpikeCB( client.get_response( ) );
    }

    return result;
  }

} // namespace simil
