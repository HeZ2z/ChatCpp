#!/usr/bin/env python3
"""
ChatCpp Test Runner with Xray Integration

This script runs the test suite and generates Xray-compatible test reports.
It can be integrated with Jira/Xray for automated test management.
"""

import os
import sys
import json
import subprocess
import datetime
from pathlib import Path
import xml.etree.ElementTree as ET

class XrayTestRunner:
    def __init__(self, project_root):
        self.project_root = Path(project_root)
        self.build_dir = self.project_root / "build"
        self.test_executable = self.build_dir / "chat_tests"
        self.test_management_dir = self.project_root / "test-management"
        self.reports_dir = self.test_management_dir / "reports"

        # Create reports directory if it doesn't exist
        self.reports_dir.mkdir(parents=True, exist_ok=True)

    def build_project(self):
        """Build the project using CMake"""
        print("Building project...")

        # Create build directory if it doesn't exist
        self.build_dir.mkdir(exist_ok=True)

        # Run CMake configure
        result = subprocess.run(
            ["cmake", ".."],
            cwd=self.build_dir,
            capture_output=True,
            text=True
        )

        if result.returncode != 0:
            print(f"CMake configure failed: {result.stderr}")
            return False

        # Run make/build
        result = subprocess.run(
            ["cmake", "--build", "."],
            cwd=self.build_dir,
            capture_output=True,
            text=True
        )

        if result.returncode != 0:
            print(f"Build failed: {result.stderr}")
            return False

        print("Build successful")
        return True

    def run_tests(self):
        """Run the test suite and capture results"""
        print("Running tests...")

        if not self.test_executable.exists():
            print(f"Test executable not found: {self.test_executable}")
            return None

        # Run tests with Google Test output format
        result = subprocess.run(
            [str(self.test_executable), "--gtest_output=xml:test_results.xml"],
            cwd=self.build_dir,
            capture_output=True,
            text=True
        )

        # Move test results to reports directory
        test_results_file = self.build_dir / "test_results.xml"
        if test_results_file.exists():
            target_file = self.reports_dir / f"test_results_{datetime.datetime.now().strftime('%Y%m%d_%H%M%S')}.xml"
            test_results_file.rename(target_file)
            print(f"Test results saved to: {target_file}")
            return target_file

        return None

    def parse_test_results(self, results_file):
        """Parse JUnit XML test results"""
        if not results_file or not results_file.exists():
            return None

        try:
            tree = ET.parse(results_file)
            root = tree.getroot()

            results = {
                "total": 0,
                "passed": 0,
                "failed": 0,
                "skipped": 0,
                "tests": []
            }

            for testsuite in root:
                results["total"] += int(testsuite.get("tests", 0))
                results["failed"] += int(testsuite.get("failures", 0))
                results["skipped"] += int(testsuite.get("skipped", 0))

                for testcase in testsuite:
                    test_result = {
                        "name": testcase.get("name"),
                        "classname": testcase.get("classname"),
                        "time": float(testcase.get("time", 0)),
                        "status": "passed",
                        "failure": None
                    }

                    failure = testcase.find("failure")
                    if failure is not None:
                        test_result["status"] = "failed"
                        test_result["failure"] = failure.text

                    results["tests"].append(test_result)

            results["passed"] = results["total"] - results["failed"] - results["skipped"]
            return results

        except Exception as e:
            print(f"Error parsing test results: {e}")
            return None

    def generate_xray_report(self, test_results, test_plan_key="CHATCPP-TP-001"):
        """Generate Xray-compatible JSON report"""
        execution_key = f"CHATCPP-TE-{datetime.datetime.now().strftime('%Y%m%d%H%M%S')}"

        report = {
            "testExecutionKey": execution_key,
            "info": {
                "summary": f"Automated Test Execution - {datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')}",
                "description": "Automated test execution via CI pipeline",
                "testPlanKey": test_plan_key,
                "testEnvironments": ["CI Pipeline"]
            },
            "tests": []
        }

        # Map test names to Xray test case keys
        test_case_mapping = {
            "CreateMessage": "CHATCPP-TC-001",
            "ToString": "CHATCPP-TC-002",
            "FromString": "CHATCPP-TC-003",
            "InvalidString": "CHATCPP-TC-004",
            "TimestampUpdate": "CHATCPP-TC-005"
        }

        for test in test_results["tests"]:
            test_key = test_case_mapping.get(test["name"], f"CHATCPP-TC-{test['name']}")

            xray_test = {
                "testKey": test_key,
                "start": datetime.datetime.now().isoformat(),
                "finish": (datetime.datetime.now() + datetime.timedelta(seconds=test["time"])).isoformat(),
                "comment": f"Execution time: {test['time']}s",
                "status": "PASSED" if test["status"] == "passed" else "FAILED"
            }

            if test["failure"]:
                xray_test["comment"] += f"\nFailure: {test['failure']}"

            report["tests"].append(xray_test)

        return report

    def save_xray_report(self, report):
        """Save Xray report to file"""
        timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
        report_file = self.reports_dir / f"xray_report_{timestamp}.json"

        with open(report_file, 'w', encoding='utf-8') as f:
            json.dump(report, f, indent=2, ensure_ascii=False)

        print(f"Xray report saved to: {report_file}")
        return report_file

    def run(self):
        """Main execution method"""
        print("Starting Xray Test Runner...")

        # Build project
        if not self.build_project():
            sys.exit(1)

        # Run tests
        results_file = self.run_tests()
        if not results_file:
            sys.exit(1)

        # Parse results
        test_results = self.parse_test_results(results_file)
        if not test_results:
            sys.exit(1)

        # Generate Xray report
        xray_report = self.generate_xray_report(test_results)
        report_file = self.save_xray_report(xray_report)

        # Print summary
        print("\nTest Execution Summary:")
        print(f"Total: {test_results['total']}")
        print(f"Passed: {test_results['passed']}")
        print(f"Failed: {test_results['failed']}")
        print(f"Skipped: {test_results['skipped']}")

        if test_results['failed'] > 0:
            print("\nFailed tests:")
            for test in test_results['tests']:
                if test['status'] == 'failed':
                    print(f"  - {test['classname']}.{test['name']}: {test['failure']}")

        return test_results['failed'] == 0

if __name__ == "__main__":
    # Get project root directory
    script_dir = Path(__file__).parent
    project_root = script_dir.parent

    runner = XrayTestRunner(project_root)
    success = runner.run()

    sys.exit(0 if success else 1)
