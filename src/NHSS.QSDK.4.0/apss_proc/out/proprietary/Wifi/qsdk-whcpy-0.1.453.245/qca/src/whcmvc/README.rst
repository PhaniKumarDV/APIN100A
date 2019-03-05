whcmvc
=======

This package contains python code that provides a Model-View-Controller
framework on which different implementations of GUIs can be built.

Installation
============

Pre-requisites
--------------

The recommended practice is to install this package in a Python `virtual
environment`_. After activating the virtual environment, first make sure
you have the latest versions of pip and setuptools:

::

    pip install --upgrade pip setuptools

Installing
----------

This package can be installed directly from its tarball as shown below:

::

    pip install whcmvc-<version>.tar.gz

Note that this package depends on the *whcdiag* package which must already
be installed in the virtual environment or be installable from a
PyPi server for the above install to complete successfully.

Alternatively, the package can be installed from the source tree using the
following command:

::

    python setup.py install

Making Changes to the Package
=============================

The sections below provide some pointers to those looking to modify the package
itself.

Development Setup
-----------------

To make this package available to you for development whereby you can
edit the files directly in the source directory, set up a `virtual
environment`_ and then activate the development mode:

::

    python setup.py develop

Development Style Guidelines
----------------------------

All new code (including test code) should follow `PEP-8`_. To check
whether the code is PEP-8 compliant, use the following command:

::

    python setup.py flake8

Note that we are allowing for lines up to 100 characters in length.

Additionally, new functions and classes should include doc strings
as described in the `Google Python Style Guide`_. There is no automated
tool for checking compliance though.

Development Testing
-------------------

We are using `nose`_ for testing. To initiate the full tests, after
you've naturally setup your virtualenv and installed the required
packages, you'd run:

::

    pip install nose coverage  # only needed one time
    python setup.py nosetests

This will run the tests and report any failures along with the code
coverage information. If you want to run nose by hand, use the following
command:

::

    nosetests -v --exe --with-coverage --cover-package=whc --cover-branches

You should hopefully see all "ok" messages. If not, you can visit the
individual test file to see how and what it was trying to do within the
``tests`` directory.

.. _virtual environment: http://docs.python-guide.org/en/latest/dev/virtualenvs/

.. _PEP-8: https://www.python.org/dev/peps/pep-0008/

.. _Google Python Style Guide: https://google-styleguide.googlecode.com/svn/trunk/pyguide.html#Comments

.. _nose: https://nose.readthedocs.org/en/latest/
