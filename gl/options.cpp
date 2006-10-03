/***** BEGIN LICENSE BLOCK *****

BSD License

Copyright (c) 2005, NIF File Format Library and Tools
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the NIF File Format Library and Tools projectmay not be
   used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

***** END LICENCE BLOCK *****/

#include "options.h"

#include <QAction>
#include <QButtonGroup>
#include <QCheckBox>
#include <QColor>
#include <QDialog>
#include <QEvent>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSettings>
#include <QSpinBox>
#include <QStack>
#include <QTimer>

#include "widgets/colorwheel.h"

class GroupBox : public QGroupBox
{
	QStack<QBoxLayout*> lay;
public:
	GroupBox( const QString & title, Qt::Orientation o ) : QGroupBox( title )
	{
		lay.push( new QBoxLayout( o2d( o ), this ) );
	}
	
	void addWidget( QWidget * widget, int stretch = 0, Qt::Alignment alignment = 0 )
	{
		lay.top()->addWidget( widget, stretch, alignment );
	}
	
	QWidget * pushLayout( const QString & name, Qt::Orientation o, int stretch = 0, Qt::Alignment alignment = 0 )
	{
		QGroupBox * grp = new QGroupBox( name );
		lay.top()->addWidget( grp, stretch, alignment );
		QBoxLayout * l = new QBoxLayout( o2d( o ) );
		grp->setLayout( l );
		lay.push( l );
		return grp;
	}
	
	void pushLayout( Qt::Orientation o )
	{
		QBoxLayout * l = new QBoxLayout( o2d( o ) );
		lay.top()->addLayout( l );
		lay.push( l );
	}
	
	void popLayout()
	{
		if ( lay.count() > 1 )
			lay.pop();
	}
	
	QBoxLayout::Direction o2d( Qt::Orientation o )
	{
		switch ( o )
		{
			case Qt::Vertical:
				return QBoxLayout::TopToBottom;
			case Qt::Horizontal:
			default:
				return QBoxLayout::LeftToRight;
		}
	}
};

GLOptions * GLOptions::get()
{
	static GLOptions * options = new GLOptions();
	return options;
}

QList<QAction*> GLOptions::actions()
{
	GLOptions * opts = get();
	return QList<QAction*>() << opts->aDrawAxes << opts->aDrawNodes << opts->aDrawHavok << opts->aDrawFurn << opts->aDrawHidden << opts->aDrawStats << opts->aSettings;
}

GLOptions::GLOptions()
{
	QSettings cfg( "NifTools", "NifSkope" );
	cfg.beginGroup( "Render Settings" );
	
	aDrawAxes = new QAction( "Draw &Axes", this );
	aDrawAxes->setToolTip( "draw xyz-Axes" );
	aDrawAxes->setCheckable( true );
	aDrawAxes->setChecked( cfg.value( "Draw AXes", true ).toBool() );
	connect( aDrawAxes, SIGNAL( toggled( bool ) ), this, SIGNAL( sigChanged() ) );
	
	aDrawNodes = new QAction( "Draw &Nodes", this );
	aDrawNodes->setToolTip( "draw bones/nodes" );
	aDrawNodes->setCheckable( true );
	aDrawNodes->setChecked( cfg.value( "Draw Nodes", true ).toBool() );
	connect( aDrawNodes, SIGNAL( toggled( bool ) ), this, SIGNAL( sigChanged() ) );
	
	aDrawHavok = new QAction( "Draw &Havok", this );
	aDrawHavok->setToolTip( "draw the havok shapes" );
	aDrawHavok->setCheckable( true );
	aDrawHavok->setChecked( cfg.value( "Draw Collision Geometry", true ).toBool() );
	connect( aDrawHavok, SIGNAL( toggled( bool ) ), this, SIGNAL( sigChanged() ) );

	aDrawFurn = new QAction( "Draw &Furniture", this );
	aDrawFurn->setToolTip( "draw the furniture markers" );
	aDrawFurn->setCheckable( true );
	aDrawFurn->setChecked( cfg.value( "Draw Furniture Markers", true ).toBool() );
	connect( aDrawFurn, SIGNAL( toggled( bool ) ), this, SIGNAL( sigChanged() ) );
	
	aDrawHidden = new QAction( "Show Hid&den", this );
	aDrawHidden->setToolTip( "always draw nodes and meshes" );
	aDrawHidden->setCheckable( true );
	aDrawHidden->setChecked( cfg.value( "Show Hidden Objects", false ).toBool() );
	connect( aDrawHidden, SIGNAL( toggled( bool ) ), this, SIGNAL( sigChanged() ) );
	
	aDrawStats = new QAction( "Show S&tats", this );
	aDrawStats->setToolTip( "display some statistics about the selected node" );
	aDrawStats->setCheckable( true );
	aDrawStats->setChecked( cfg.value( "Show Stats", false ).toBool() );
	connect( aDrawStats, SIGNAL( toggled( bool ) ), this, SIGNAL( sigChanged() ) );
	
	
	dialog = new GroupBox( "", Qt::Vertical );
	dialog->setWindowTitle(  "Render Settings"  );
	dialog->setWindowFlags( Qt::Tool | Qt::WindowStaysOnTopHint );
	dialog->installEventFilter( this );
	
	aSettings = new QAction( "&Settings...", this );
	aSettings->setToolTip( "show the settings dialog" );
	connect( aSettings, SIGNAL( triggered() ), dialog, SLOT( show() ) );
	
	
	dialog->pushLayout( Qt::Horizontal );
	dialog->pushLayout( "Render", Qt::Vertical );

	dialog->addWidget( AntiAlias = new QCheckBox( "&Anti Aliasing" ) );
	AntiAlias->setToolTip( "Enable anti aliasing if available.<br>You'll need to restart NifSkope for this setting to take effect." );
	AntiAlias->setChecked( cfg.value( "Anti Aliasing", true ).toBool() );
	connect( AntiAlias, SIGNAL( toggled( bool ) ), this, SIGNAL( sigChanged() ) );
	
	dialog->addWidget( Textures = new QCheckBox( "&Textures" ) );
	Textures->setToolTip( "Enable textures" );
	Textures->setChecked( cfg.value( "Texturing", true ).toBool() );
	connect( Textures, SIGNAL( toggled( bool ) ), this, SIGNAL( sigChanged() ) );
	
	dialog->addWidget( Shaders = new QCheckBox( "&Shaders" ) );
	Shaders->setToolTip( "Enable Shaders" );
	Shaders->setChecked( cfg.value( "Enable Shaders", true ).toBool() );
	connect( Shaders, SIGNAL( toggled( bool ) ), this, SIGNAL( sigChanged() ) );
	
	
	dialog->popLayout();
	dialog->pushLayout( "Up Axis", Qt::Vertical );
	
	
	dialog->addWidget( AxisX = new QRadioButton( "X" ) );
	dialog->addWidget( AxisY = new QRadioButton( "Y" ) );
	dialog->addWidget( AxisZ = new QRadioButton( "Z" ) );
	
	QButtonGroup * btgrp = new QButtonGroup( this );
	btgrp->addButton( AxisX );
	btgrp->addButton( AxisY );
	btgrp->addButton( AxisZ );
	btgrp->setExclusive( true );
	
	QString upax = cfg.value( "Up Axis", "Z" ).toString();
	if ( upax == "X" )
		AxisX->setChecked( true );
	else if ( upax == "Y" )
		AxisY->setChecked( true );
	else
		AxisZ->setChecked( true );
		
	connect( AxisX, SIGNAL( toggled( bool ) ), this, SIGNAL( sigChanged() ) );
	connect( AxisY, SIGNAL( toggled( bool ) ), this, SIGNAL( sigChanged() ) );
	connect( AxisZ, SIGNAL( toggled( bool ) ), this, SIGNAL( sigChanged() ) );
	
	
	dialog->popLayout();
	dialog->pushLayout( "Culling", Qt::Vertical, 1 );
	
	dialog->addWidget( CullNoTex = new QCheckBox( "Cull &Non Textured" ) );
	CullNoTex->setToolTip( "Hide all meshes without textures" );
	CullNoTex->setChecked( cfg.value( "Cull Non Textured", false ).toBool() );
	connect( CullNoTex, SIGNAL( toggled( bool ) ), this, SIGNAL( sigChanged() ) );
	
	dialog->addWidget( CullByID = new QCheckBox( "&Cull Nodes by Name" ) );
	CullByID->setToolTip( "Enabling this option hides some special nodes and meshes" );
	CullByID->setChecked( cfg.value( "Cull Nodes By Name", false ).toBool() );
	connect( CullByID, SIGNAL( toggled( bool ) ), this, SIGNAL( sigChanged() ) );
	
	dialog->addWidget( CullExpr = new QLineEdit( cfg.value( "Cull Expression", "^collidee|^shadowcaster|^\\!LoD_cullme|^footprint" ).toString() ) );
	CullExpr->setToolTip( "Enter a regular expression. Nodes which names match the expression will be hidden" );
	CullExpr->setEnabled( CullByID->isChecked() );
	connect( CullExpr, SIGNAL( textChanged( const QString & ) ), this, SIGNAL( sigChanged() ) );
	connect( CullByID, SIGNAL( toggled( bool ) ), CullExpr, SLOT( setEnabled( bool ) ) );
	

	dialog->popLayout();
	dialog->popLayout();
	dialog->pushLayout( "Light", Qt::Vertical );
	dialog->pushLayout( Qt::Horizontal );
	
	
	cfg.beginGroup( "Light0" );
	
	QStringList lightNames( QStringList() << "Ambient" << "Diffuse" << "Specular" );
	QList<QColor> lightDefaults( QList<QColor>() << QColor::fromRgbF( .4, .4, .4 ) << QColor::fromRgbF( .8, .8, .8 ) << QColor::fromRgbF( 1, 1, 1 ) );
	for ( int l = 0; l < 3; l++ )
	{
		ColorWheel * wheel = new ColorWheel( cfg.value( lightNames[l], lightDefaults[l] ).value<QColor>() );
		wheel->setSizeHint( QSize( 105, 105 ) );
		connect( wheel, SIGNAL( sigColorEdited( const QColor & ) ), this, SIGNAL( sigChanged() ) );
		LightColor[l] = wheel;
		
		dialog->pushLayout( lightNames[l], Qt::Vertical );
		dialog->addWidget( wheel );
		dialog->popLayout();
	}
	
	dialog->popLayout();
	dialog->pushLayout( Qt::Horizontal );
	
	dialog->addWidget( LightFrontal = new QCheckBox( "Frontal" ), 0 );
	LightFrontal->setToolTip( "Light will remain fixed at the camera location." );
	LightFrontal->setChecked( cfg.value( "Frontal", true ).toBool() );
	connect( LightFrontal, SIGNAL( toggled( bool ) ), this, SIGNAL( sigChanged() ) );
	
	QWidget * pos = dialog->pushLayout( "Position", Qt::Horizontal, 1 );
	pos->setDisabled( LightFrontal->isChecked() );
	connect( LightFrontal, SIGNAL( toggled( bool ) ), pos, SLOT( setDisabled( bool ) ) );
	
	dialog->addWidget( new QLabel( "Declination" ) );
	dialog->addWidget( LightDeclination = new QSpinBox, 1 );
	LightDeclination->setMinimum( -180 );
	LightDeclination->setMaximum( +180 );
	LightDeclination->setSingleStep( 5 );
	LightDeclination->setWrapping( true );
	LightDeclination->setValue( cfg.value( "Declination", 0 ).toInt() );
	connect( LightDeclination, SIGNAL( valueChanged( int ) ), this, SIGNAL( sigChanged() ) );
	
	dialog->addWidget( new QLabel( "Planar Angle" ) );
	dialog->addWidget( LightPlanarAngle = new QSpinBox, 1 );
	LightPlanarAngle->setMinimum( -180 );
	LightPlanarAngle->setMaximum( +180 );
	LightPlanarAngle->setSingleStep( 5 );
	LightPlanarAngle->setWrapping( true );
	LightPlanarAngle->setValue( cfg.value( "Planar Angle", 0 ).toInt() );
	connect( LightPlanarAngle, SIGNAL( valueChanged( int ) ), this, SIGNAL( sigChanged() ) );
	
	cfg.endGroup();
	
	dialog->popLayout();
	dialog->pushLayout( "Presets", Qt::Horizontal );
	
	QButtonGroup * grp = new QButtonGroup( this );
	connect( grp, SIGNAL( buttonClicked( int ) ), this, SLOT( activateLightPreset( int ) ) );
	int psid = 0;
	foreach ( QString psname, QStringList() << "Sunny Day" << "Dark Night" )
	{
		QPushButton * bt = new QPushButton( psname );
		grp->addButton( bt, psid++ );
		dialog->addWidget( bt );
	}
	
	dialog->popLayout();
	dialog->popLayout();
	dialog->popLayout();
	dialog->pushLayout( "Colors", Qt::Horizontal );
	
	QStringList colorNames( QStringList() << "Background" << "Foreground" << "Highlight" );
	QList<QColor> colorDefaults( QList<QColor>() << QColor::fromRgb( 0, 0, 0 ) << QColor::fromRgb( 255, 255, 255 ) << QColor::fromRgb( 255, 255, 0 ) );
	for ( int c = 0; c < 3; c++ )
	{
		ColorWheel * wheel = new ColorWheel( cfg.value( colorNames[c], colorDefaults[c] ).value<QColor>() );
		wheel->setSizeHint( QSize( 105, 105 ) );
		connect( wheel, SIGNAL( sigColorEdited( const QColor & ) ), this, SIGNAL( sigChanged() ) );
		colors[ c ] = wheel;
		
		dialog->pushLayout( colorNames[c], Qt::Vertical );
		dialog->addWidget( wheel );
		dialog->popLayout();
	}

	dialog->popLayout();
	
	tSave = new QTimer( this );
	tSave->setInterval( 5000 );
	tSave->setSingleShot( true );
	connect( this, SIGNAL( sigChanged() ), tSave, SLOT( start() ) );
	connect( tSave, SIGNAL( timeout() ), this, SLOT( save() ) );
}

GLOptions::~GLOptions()
{
	if ( tSave->isActive() )
		save();
}

bool GLOptions::eventFilter( QObject * o, QEvent * e )
{
	if ( o == dialog && e->type() == QEvent::Close && tSave->isActive() )
	{
		save();
	}
	return false;
}

void GLOptions::save()
{
	tSave->stop();
	
	QSettings cfg( "NifTools", "NifSkope" );
	cfg.beginGroup( "Render Settings" );
	
	cfg.setValue( "Draw Axes", drawAxes() );
	cfg.setValue( "Draw Nodes", drawNodes() );
	cfg.setValue( "Draw Collision Geometry", drawHavok() );
	cfg.setValue( "Draw Furniture Markers", drawFurn() );
	cfg.setValue( "Show Hidden Objects", drawHidden() );
	cfg.setValue( "Show Stats", drawStats() );

	cfg.setValue( "Background", bgColor() );
	cfg.setValue( "Foreground", nlColor() );
	cfg.setValue( "Highlight", hlColor() );
	
	cfg.setValue( "Anti Aliasing", antialias() );
	cfg.setValue( "Texturing", texturing() );
	cfg.setValue( "Enable Shaders", shaders() );
	
	cfg.setValue( "Cull Nodes By Name", CullByID->isChecked() );
	cfg.setValue( "Cull Expression", CullExpr->text() );
	cfg.setValue( "Cull Non Textured", onlyTextured() );
	
	cfg.setValue( "Up Axis", AxisX->isChecked() ? "X" : AxisY->isChecked() ? "Y" : "Z" );
	
	cfg.beginGroup( "Light0" );
	cfg.setValue( "Ambient", ambient() );
	cfg.setValue( "Diffuse", diffuse() );
	cfg.setValue( "Specular", specular() );
	cfg.setValue( "Frontal", lightFrontal() );
	cfg.setValue( "Declination", lightDeclination() );
	cfg.setValue( "Planar Angle", lightPlanarAngle() );
	cfg.endGroup();
}

void GLOptions::activateLightPreset( int id )
{
	switch ( id )
	{
		case 0: // sunny day
			LightColor[0]->setColor( QColor::fromRgbF( 0.4, 0.4, 0.4 ) );
			LightColor[1]->setColor( QColor::fromRgbF( 0.8, 0.8, 0.8 ) );
			LightColor[2]->setColor( QColor::fromRgbF( 1.0, 1.0, 1.0 ) );
			break;
		case 1: // dark night
			LightColor[0]->setColor( QColor::fromRgbF( 0.2, 0.2, 0.25 ) );
			LightColor[1]->setColor( QColor::fromRgbF( 0.1, 0.1, 0.1 ) );
			LightColor[2]->setColor( QColor::fromRgbF( 0.1, 0.1, 0.1 ) );
			break;
		default:
			return;
	}
	emit sigChanged();
}


GLOptions::Axis GLOptions::upAxis()
{
	return get()->AxisX->isChecked() ? XAxis : get()->AxisY->isChecked() ? YAxis : ZAxis;
}

bool GLOptions::antialias()
{
	return get()->AntiAlias->isChecked();
}

bool GLOptions::texturing()
{
	return get()->Textures->isChecked();
}

bool GLOptions::shaders()
{
	return get()->Shaders->isChecked();
}


bool GLOptions::drawAxes()
{
	return get()->aDrawAxes->isChecked();
}

bool GLOptions::drawNodes()
{
	return get()->aDrawNodes->isChecked();
}

bool GLOptions::drawHavok()
{
	return get()->aDrawHavok->isChecked();
}

bool GLOptions::drawFurn()
{
	return get()->aDrawFurn->isChecked();
}

bool GLOptions::drawHidden()
{
	return get()->aDrawHidden->isChecked();
}

bool GLOptions::drawStats()
{
	return get()->aDrawStats->isChecked();
}

bool GLOptions::benchmark()
{
	return false;
}


QColor GLOptions::bgColor()
{
	return get()->colors[ 0 ]->getColor();
}

QColor GLOptions::nlColor()
{
	return get()->colors[ 1 ]->getColor();
}

QColor GLOptions::hlColor()
{
	return get()->colors[ 2 ]->getColor();
}


QRegExp GLOptions::cullExpression()
{
	return get()->CullByID->isChecked() ? QRegExp( get()->CullExpr->text() ) : QRegExp();
}

bool GLOptions::onlyTextured()
{
	return get()->CullNoTex->isChecked();
}


QColor GLOptions::ambient()
{
	return get()->LightColor[ 0 ]->getColor();
}

QColor GLOptions::diffuse()
{
	return get()->LightColor[ 1 ]->getColor();
}

QColor GLOptions::specular()
{
	return get()->LightColor[ 2 ]->getColor();
}

bool GLOptions::lightFrontal()
{
	return get()->LightFrontal->isChecked();
}

int GLOptions::lightDeclination()
{
	return get()->LightDeclination->value();
}

int GLOptions::lightPlanarAngle()
{
	return get()->LightPlanarAngle->value();
}

