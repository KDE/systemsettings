#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2024 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: MIT

import subprocess
import time
import unittest
from typing import Final

from appium import webdriver
from appium.options.common.base import AppiumOptions
from appium.webdriver.common.appiumby import AppiumBy
from selenium.webdriver.common.action_chains import ActionChains
from selenium.webdriver.common.keys import Keys

KDE_VERSION: Final = 6


class AppTest(unittest.TestCase):
    """
    Tests for launching systemsettings
    """

    driver: webdriver.Remote

    @classmethod
    def setUpClass(cls) -> None:
        options = AppiumOptions()
        options.set_capability("app", "systemsettings")
        options.set_capability("timeouts", {'implicit': 10000})
        options.set_capability("environ", {
            "LC_ALL": "en_US.UTF-8",
            "QT_LOGGING_RULES": "qt.accessibility.atspi.warning=false;qt.qpa.wayland.warning=false;kf.auth.warning=false;kf.plasma.core.warning=false;kf.windowsystem.warning=false;kf.kirigami.platform.warning=false;org.kde.plasma.kcm_feedback.warning=false",
        })
        cls.driver = webdriver.Remote(command_executor='http://127.0.0.1:4723', options=options)

    def tearDown(self) -> None:
        """
        Take screenshot when the current test fails
        """
        if not self._outcome.result.wasSuccessful():
            self.driver.get_screenshot_as_file(f"failed_test_shot_apptest_#{self.id()}.png")

    @classmethod
    def tearDownClass(cls) -> None:
        """
        Make sure to terminate the driver again, lest it dangles.
        """
        subprocess.check_call([f"kquitapp{KDE_VERSION}", "systemsettings"])
        for _ in range(10):
            try:
                subprocess.check_call(["pidof", "systemsettings"])
            except subprocess.CalledProcessError:
                break
            time.sleep(1)
        cls.driver.quit()

    def test_0_open(self) -> None:
        """
        Launch systemsettings
        """
        self.driver.find_element(by=AppiumBy.XPATH, value="//push_button_menu[@name='Show menu']")

    def test_1_search(self) -> None:
        """
        Search and keyboard navigation
        """
        self.driver.find_element(by=AppiumBy.NAME, value="Search").send_keys("User Feed")
        self.driver.find_element(by=AppiumBy.XPATH, value="//list_item[@name='User Feedback' and not(contains(@states, 'focused'))]")
        ActionChains(self.driver).send_keys(Keys.DOWN).perform()
        self.driver.find_element(by=AppiumBy.XPATH, value="//list_item[@name='User Feedback' and contains(@states, 'focused')]")
        ActionChains(self.driver).send_keys(Keys.RETURN).perform()
        self.driver.find_element(by=AppiumBy.NAME, value="Plasma:")


if __name__ == '__main__':
    unittest.main()
