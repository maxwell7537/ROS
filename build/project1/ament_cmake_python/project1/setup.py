from setuptools import find_packages
from setuptools import setup

setup(
    name='project1',
    version='0.0.0',
    packages=find_packages(
        include=('project1', 'project1.*')),
)
