/*
 * @file  LoadblueConfigData.h
 * @brief
 * @author Aaron Sújar <aaron.sujar@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#include "LoaderBlueConfigData.h"


namespace simil
{
  LoaderBlueConfigData::LoaderBlueConfigData( )
    : LoaderSimData( )
    , _blueConfig( nullptr )
  {
  }
  LoaderBlueConfigData::~LoaderBlueConfigData( )
  {
    if ( _blueConfig != nullptr )
      delete _blueConfig;
  }

  Network *LoaderBlueConfigData::loadNetwork(const std::string &filePath_,
                                           const std::string &aux)
  {

    Network * _network = new Network();

    if (_blueConfig == nullptr)
    {
      _blueConfig = new brion::BlueConfig(filePath_);
    }

    _network->setDataType(TBlueConfig);

    brion::Targets targets = _blueConfig->getTargets();

    brain::Circuit *circuit = new brain::Circuit(*_blueConfig);

    if (!aux.empty())
      _network->setGids(brion::Target::parse(targets, aux));
    else
      _network->setGids(circuit->getGIDs());

    _network->setPositions(circuit->getPositions(_network->gids()));

    delete circuit;

    return _network;
  }

  SimulationData*
    LoaderBlueConfigData::loadSimulationData( const std::string& filePath_,
                                              const std::string&  )
  {
    SpikeData* simulationdata = new SpikeData( );

    if ( _blueConfig == nullptr )
    {
      _blueConfig = new brion::BlueConfig( filePath_ );
    }

    brain::SpikeReportReader spikeReport( _blueConfig->getSpikeSource( ) );
    simulationdata->setSimulationType( TSimSpikes );
    simulationdata->setSpikes(
      spikeReport.getSpikes( 0, spikeReport.getEndTime( ) ) );
    simulationdata->setStartTime( 0.0f );
    simulationdata->setEndTime( spikeReport.getEndTime( ) );



    return simulationdata;
  }

} // namespace simil
