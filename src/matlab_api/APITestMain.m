% APITestMain
clc
clear all
close all

%% setup globals
global TEST_TOL;
TEST_TOL = 1e-3;    % used in functions for delta assessment

global m_vspfname_for_vspaerotests;
m_vspfname_for_vspaerotests = 'apitest_TestVSPAero.vsp3';

global m_vspfname_for_vspaerofunctionalitytests;
m_vspfname_for_vspaerofunctionalitytests = 'apitest_TestVSPAeroFunctionality.vsp3';

global VSPAERO_PATH
VSPAERO_PATH = [pwd '\..\'];

%% Execute tests
% % Simple run syntax
% results = run(APITestSuiteTest);
% rt = table(results)

% Complex runner with progress display
import matlab.unittest.TestRunner
import matlab.unittest.TestSuite
% import matlab.unittest.plugins.TestRunProgressPlugin
% import matlab.unittest.plugins.FailureDiagnosticsPlugin

% Create silent test runner and add plug-in to display progress
runner = TestRunner.withTextOutput;
% runner.addPlugin(TestRunProgressPlugin.withVerbosity(matlab.unittest.Verbosity.Detailed))
% runner.addPlugin(FailureDiagnosticsPlugin)

% Run general tests
suite = TestSuite.fromFile('APITestSuite_test.m');
result = run(runner,suite)

% Run VSPAERO unit tests
suiteVSPAERO = TestSuite.fromFile('APITestSuiteVSPAERO_test.m');
result = run(runner,suiteVSPAERO)
