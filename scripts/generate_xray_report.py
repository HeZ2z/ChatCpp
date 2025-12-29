#!/usr/bin/env python3
"""
Xray Report Generator for ChatCpp

This script generates detailed Xray-compatible test reports from test execution results.
"""

import json
import argparse
from pathlib import Path
from datetime import datetime, timedelta
import xml.etree.ElementTree as ET

class XrayReportGenerator:
    def __init__(self, config_file=None):
        self.config_file = config_file or Path(__file__).parent.parent / "config" / "jira-xray-config.json"
        self.config = self.load_config()

    def load_config(self):
        """Load configuration from JSON file"""
        try:
            with open(self.config_file, 'r', encoding='utf-8') as f:
                return json.load(f)
        except FileNotFoundError:
            print(f"Configuration file not found: {self.config_file}")
            return {}
        except json.JSONDecodeError as e:
            print(f"Error parsing configuration: {e}")
            return {}

    def parse_junit_xml(self, xml_file):
        """Parse JUnit XML test results"""
        try:
            tree = ET.parse(xml_file)
            root = tree.getroot()

            results = {
                "testsuites": [],
                "summary": {
                    "total": 0,
                    "passed": 0,
                    "failed": 0,
                    "skipped": 0,
                    "errors": 0,
                    "time": 0.0
                }
            }

            for testsuite in root:
                suite = {
                    "name": testsuite.get("name", ""),
                    "tests": int(testsuite.get("tests", 0)),
                    "failures": int(testsuite.get("failures", 0)),
                    "errors": int(testsuite.get("errors", 0)),
                    "skipped": int(testsuite.get("skipped", 0)),
                    "time": float(testsuite.get("time", 0)),
                    "timestamp": testsuite.get("timestamp", ""),
                    "tests": []
                }

                for testcase in testsuite:
                    test = {
                        "name": testcase.get("name", ""),
                        "classname": testcase.get("classname", ""),
                        "time": float(testcase.get("time", 0)),
                        "status": "passed",
                        "failure": None,
                        "error": None,
                        "skipped": None
                    }

                    # Check for failure
                    failure = testcase.find("failure")
                    if failure is not None:
                        test["status"] = "failed"
                        test["failure"] = {
                            "message": failure.get("message", ""),
                            "type": failure.get("type", ""),
                            "content": failure.text or ""
                        }

                    # Check for error
                    error = testcase.find("error")
                    if error is not None:
                        test["status"] = "error"
                        test["error"] = {
                            "message": error.get("message", ""),
                            "type": error.get("type", ""),
                            "content": error.text or ""
                        }

                    # Check for skipped
                    skipped = testcase.find("skipped")
                    if skipped is not None:
                        test["status"] = "skipped"
                        test["skipped"] = skipped.text or ""

                    suite["tests"].append(test)

                results["testsuites"].append(suite)

                # Update summary
                results["summary"]["total"] += suite["tests"]
                results["summary"]["failed"] += suite["failures"]
                results["summary"]["errors"] += suite["errors"]
                results["summary"]["skipped"] += suite["skipped"]
                results["summary"]["time"] += suite["time"]

            results["summary"]["passed"] = (
                results["summary"]["total"] -
                results["summary"]["failed"] -
                results["summary"]["errors"] -
                results["summary"]["skipped"]
            )

            return results

        except Exception as e:
            print(f"Error parsing JUnit XML: {e}")
            return None

    def map_test_to_xray_key(self, test_name):
        """Map test name to Xray test case key"""
        mapping = self.config.get("integration", {}).get("testCaseMapping", {}).get("automationMapping", {})
        return mapping.get(test_name, f"CHATCPP-TC-{test_name}")

    def generate_xray_json_report(self, junit_results, test_plan_key=None, test_environment=None):
        """Generate Xray JSON format report"""
        now = datetime.now()

        report = {
            "testExecutionKey": f"CHATCPP-TE-{now.strftime('%Y%m%d%H%M%S')}",
            "info": {
                "summary": f"Automated Test Execution - {now.strftime('%Y-%m-%d %H:%M:%S')}",
                "description": "Automated test execution via CI/CD pipeline",
                "startDate": now.isoformat(),
                "finishDate": (now + timedelta(seconds=junit_results['summary']['time'])).isoformat(),
                "testPlanKey": test_plan_key or "CHATCPP-TP-001",
                "testEnvironments": [test_environment or "CI Pipeline"]
            },
            "tests": []
        }

        test_start_time = now

        for suite in junit_results["testsuites"]:
            for test in suite["tests"]:
                test_key = self.map_test_to_xray_key(test["name"])

                xray_test = {
                    "testKey": test_key,
                    "start": test_start_time.isoformat(),
                    "finish": (test_start_time + timedelta(seconds=test["time"])).isoformat(),
                    "comment": f"Execution time: {test['time']}s\nClass: {test['classname']}",
                    "status": self.map_status_to_xray(test["status"])
                }

                # Add failure details if test failed
                if test["status"] == "failed" and test["failure"]:
                    xray_test["comment"] += f"\n\nFAILURE:\n{test['failure']['content']}"
                    if test["failure"]["message"]:
                        xray_test["comment"] += f"\nMessage: {test['failure']['message']}"

                # Add error details if test errored
                if test["status"] == "error" and test["error"]:
                    xray_test["comment"] += f"\n\nERROR:\n{test['error']['content']}"
                    if test["error"]["message"]:
                        xray_test["comment"] += f"\nMessage: {test['error']['message']}"

                report["tests"].append(xray_test)
                test_start_time += timedelta(seconds=test["time"])

        return report

    def map_status_to_xray(self, status):
        """Map test status to Xray status"""
        status_mapping = self.config.get("xray", {}).get("testStatuses", {})
        return status_mapping.get(status, status.upper())

    def generate_xray_cucumber_report(self, junit_results):
        """Generate Xray Cucumber format report (for BDD tests)"""
        # This would be used if you had Cucumber/Gherkin tests
        # For now, returning basic structure
        return []

    def save_report(self, report, output_file, format="json"):
        """Save report to file"""
        output_file = Path(output_file)

        if format == "json":
            with open(output_file, 'w', encoding='utf-8') as f:
                json.dump(report, f, indent=2, ensure_ascii=False)
        elif format == "xml":
            # Convert to XML if needed
            pass

        print(f"Report saved to: {output_file}")
        return output_file

    def generate_summary_report(self, junit_results, output_file):
        """Generate a human-readable summary report"""
        summary = junit_results["summary"]

        report = f"""# ChatCpp Test Execution Summary

Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}

## Overall Results
- **Total Tests**: {summary['total']}
- **Passed**: {summary['passed']}
- **Failed**: {summary['failed']}
- **Errors**: {summary['errors']}
- **Skipped**: {summary['skipped']}
- **Success Rate**: {((summary['passed'] / summary['total']) * 100) if summary['total'] > 0 else 0:.1f}%
- **Total Time**: {summary['time']:.2f}s

## Test Suite Details
"""

        for suite in junit_results["testsuites"]:
            report += f"""
### {suite['name']}
- Tests: {suite['tests']}
- Failures: {suite['failures']}
- Errors: {suite['errors']}
- Skipped: {suite['skipped']}
- Time: {suite['time']:.2f}s
"""

            # Add failed tests details
            failed_tests = [t for t in suite["tests"] if t["status"] in ["failed", "error"]]
            if failed_tests:
                report += "\n**Failed Tests:**\n"
                for test in failed_tests:
                    report += f"- {test['classname']}.{test['name']}: {test['status']}\n"
                    if test.get("failure"):
                        report += f"  - {test['failure']['message']}\n"

        with open(output_file, 'w', encoding='utf-8') as f:
            f.write(report)

        print(f"Summary report saved to: {output_file}")
        return output_file

def main():
    parser = argparse.ArgumentParser(description="Generate Xray-compatible test reports")
    parser.add_argument("input_xml", help="JUnit XML test results file")
    parser.add_argument("--output-dir", default="test-management/reports",
                       help="Output directory for reports")
    parser.add_argument("--test-plan", default="CHATCPP-TP-001",
                       help="Xray test plan key")
    parser.add_argument("--environment", default="CI Pipeline",
                       help="Test environment")
    parser.add_argument("--format", choices=["json", "cucumber"], default="json",
                       help="Report format")

    args = parser.parse_args()

    generator = XrayReportGenerator()

    # Parse JUnit results
    junit_results = generator.parse_junit_xml(args.input_xml)
    if not junit_results:
        return 1

    # Create output directory
    output_dir = Path(args.output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")

    # Generate Xray JSON report
    xray_report = generator.generate_xray_json_report(
        junit_results,
        args.test_plan,
        args.environment
    )

    json_file = output_dir / f"xray_report_{timestamp}.json"
    generator.save_report(xray_report, json_file, "json")

    # Generate summary report
    summary_file = output_dir / f"test_summary_{timestamp}.md"
    generator.generate_summary_report(junit_results, summary_file)

    # Print summary to console
    summary = junit_results["summary"]
    print("
Test Execution Summary:"    print(f"Total: {summary['total']}, Passed: {summary['passed']}, Failed: {summary['failed']}")

    return 0 if summary['failed'] == 0 and summary['errors'] == 0 else 1

if __name__ == "__main__":
    exit(main())

