/*
 * VadereLauchner.cc
 *
 *  Created on: Aug 6, 2020
 *      Author: sts
 */

#include "rover/artery/traci/VadereLauchner.h"
#include "rover/artery/traci/VadereApi.h"

#include "rover/artery/traci/VadereUtils.h"

using namespace traci;
using namespace omnetpp;

namespace rover {

Define_Module(VadereLauchner);

std::pair<API*, LiteAPI*> VadereLauchner::createAPI() {
  VadereApi* api = new VadereApi();
  VadereLiteApi* liteApi = new VadereLiteApi(*api);
  return std::make_pair(api, liteApi);
}

void VadereLauchner::initializeServer(VadereLiteApi* m_lite, VadereApi* m_api) {
  // default basedir is where current network file was loaded from
  std::string basedir = cSimulation::getActiveSimulation()
                            ->getEnvir()
                            ->getConfig()
                            ->getConfigEntry("network")
                            .getBaseDirectory();
  std::string vadereScenarioPath = par("vadereScenarioPath").stdstringValue();
  std::string vadereCachePath = par("vadereCachePath").stdstringValue();

  vadere::VadereScenario scenario =
      vadere::getScenarioContent(basedir, vadereScenarioPath);

  // get scenarioHash for cache location
  std::string scenarioHash =
      m_lite->vSimulation().getScenarioHash(scenario.second);

  std::vector<vadere::VadereCache> cachePaths;
  cachePaths = vadere::getCachePaths(basedir, vadereCachePath, scenarioHash);

  vadere::SimCfg simCfg;
  simCfg.oppConfigName = cSimulation::getActiveSimulation()
                             ->getEnvir()
                             ->getConfigEx()
                             ->getVariable(CFGVAR_CONFIGNAME);
  simCfg.oppExperiment = cSimulation::getActiveSimulation()
                             ->getEnvir()
                             ->getConfigEx()
                             ->getVariable(CFGVAR_EXPERIMENT);
  simCfg.oppDateTime = cSimulation::getActiveSimulation()
                           ->getEnvir()
                           ->getConfigEx()
                           ->getVariable(CFGVAR_DATETIME);
  simCfg.oppResultRootDir = basedir + cSimulation::getActiveSimulation()
                                          ->getEnvir()
                                          ->getConfigEx()
                                          ->getVariable(CFGVAR_RESULTDIR);
  simCfg.oppIterationVariables = cSimulation::getActiveSimulation()
                                     ->getEnvir()
                                     ->getConfigEx()
                                     ->getVariable(CFGVAR_ITERATIONVARSF);
  simCfg.oppRepetition = cSimulation::getActiveSimulation()
                             ->getEnvir()
                             ->getConfigEx()
                             ->getVariable(CFGVAR_REPETITION);
  cConfigOption* vecObj = cConfigOption::find("output-vector-file");
  if (vecObj) {
    simCfg.oppOutputVecFile = cSimulation::getActiveSimulation()
                                  ->getEnvir()
                                  ->getConfig()
                                  ->getAsFilename(vecObj);
  } else {
    simCfg.oppOutputVecFile = "";
  }
  cConfigOption* scaObj = cConfigOption::find("output-scalar-file");
  if (vecObj) {
    simCfg.oppOutputScalarFile = cSimulation::getActiveSimulation()
                                     ->getEnvir()
                                     ->getConfig()
                                     ->getAsFilename(scaObj);
  } else {
    simCfg.oppOutputScalarFile = "";
  }
  int seed = par("seed").intValue();
  if (seed == -1) {
    // default seed is current repetition
    const char* seed_s = cSimulation::getActiveSimulation()
                             ->getEnvir()
                             ->getConfigEx()
                             ->getVariable(CFGVAR_RUNNUMBER);
    seed = atoi(seed_s);
  }
  simCfg.seed = seed;
  simCfg.useVadereSeed = par("useVadereSeed").boolValue();

  m_lite->vSimulation().sendSimulationConfig(simCfg);

  m_api->sendFile(scenario);
}

} /* namespace rover */
