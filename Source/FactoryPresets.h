#pragma once

#include "PluginProcessor.h"
#include <array>
#include <algorithm>
#include <cmath>

namespace FactoryPresets {
struct Core {
    const char* name;
    int mode, wave1, wave2, table, filter;
    float mix, detune, sub, cutoff, resonance;
    float attack, decay, sustain, release, filterAmount;
    int unison;
    float uniDetune, spread, drive;
    int fxProfile;
    int mod1Source, mod1Target;
    float mod1Amount;
    int mod2Source, mod2Target;
    float mod2Amount;
    int seqProfile;
};

#define CORE(n, mo, w1, w2, tb, fi, mx, de, su, cu, re, aa, dd, ss, rr, fe, un, ud, sp, dr, fx, s1, t1, a1, s2, t2, a2, sq) \
    { n, mo, w1, w2, tb, fi, mx, de, su, cu, re, aa, dd, ss, rr, fe, un, ud, sp, dr, fx, s1, t1, a1, s2, t2, a2, sq }

inline constexpr Core cores[10][10] = {
    { // LEADS
        CORE("Mono Voltage",0,0,3,0,0,.18f,4,.02f,9200,.18f,.002f,.28f,.82f,.22f,.18f,1,2,.05f,.08f,1,1,1,.12f,3,7,.10f,0),
        CORE("Nova Saw",4,0,2,3,0,.28f,20,.04f,11800,.22f,.004f,.32f,.78f,.34f,.22f,7,22,.92f,.08f,2,1,5,.14f,9,4,.12f,0),
        CORE("Glass Bite",1,3,2,4,0,.62f,5,.00f,13200,.16f,.001f,.62f,.08f,.40f,.32f,2,8,.38f,.02f,3,2,3,.18f,9,1,.16f,0),
        CORE("Sync Razor",3,0,1,3,0,.35f,12,.00f,7600,.48f,.001f,.18f,.66f,.16f,.52f,2,14,.42f,.16f,4,1,1,.12f,3,6,.10f,0),
        CORE("Acid Vector",0,1,0,5,0,.16f,7,.10f,2100,.78f,.001f,.30f,.72f,.18f,.82f,1,4,.10f,.42f,4,1,1,.35f,3,6,.12f,0),
        CORE("Hollow Brass",0,0,2,1,1,.48f,9,.00f,4100,.36f,.012f,.42f,.58f,.32f,.42f,3,13,.55f,.12f,5,1,2,.20f,9,5,.14f,0),
        CORE("Pulse Archer",3,1,0,3,0,.22f,16,.00f,6800,.58f,.001f,.24f,.54f,.22f,.66f,1,4,.16f,.22f,8,2,1,.16f,3,4,.10f,1),
        CORE("Soft Focus",0,2,3,2,0,.56f,3,.00f,8400,.12f,.018f,.34f,.62f,.46f,.16f,2,7,.42f,.00f,2,1,3,.10f,9,7,.10f,0),
        CORE("Laser Thread",3,0,3,5,2,.72f,24,.00f,10200,.24f,.001f,.14f,.18f,.28f,.24f,1,7,.20f,.34f,7,7,4,.22f,8,5,.10f,2),
        CORE("Unison Crest",4,0,1,0,0,.42f,28,.00f,9400,.28f,.006f,.48f,.76f,.48f,.20f,7,38,1.0f,.12f,14,1,5,.12f,9,1,.10f,0)
    },
    { // PADS
        CORE("Slow Bloom",0,2,3,2,0,.42f,5,.08f,5200,.20f,1.8f,2.8f,.86f,4.2f,.46f,6,18,.92f,.00f,2,1,3,.16f,5,3,.14f,0),
        CORE("Juno Veil",0,0,2,0,0,.24f,11,.04f,7200,.20f,1.1f,2.0f,.82f,3.2f,.30f,6,16,.88f,.04f,2,1,5,.10f,4,8,.12f,0),
        CORE("Glass Horizon",1,3,2,4,0,.60f,2,.00f,9700,.12f,1.5f,2.2f,.72f,4.4f,.25f,5,14,.86f,.00f,9,2,3,.13f,9,10,.14f,0),
        CORE("Analog Strings",4,0,2,1,0,.38f,17,.02f,6800,.24f,.45f,1.7f,.88f,3.0f,.42f,7,16,.96f,.05f,2,1,5,.08f,6,8,.10f,0),
        CORE("Polar Choir",1,3,3,2,1,.48f,4,.00f,3900,.28f,2.2f,3.5f,.94f,4.8f,.60f,5,10,.90f,.00f,5,1,1,.12f,9,5,.16f,0),
        CORE("Motion Cathedral",4,0,3,3,0,.68f,24,.00f,8400,.18f,2.4f,3.2f,.92f,5.0f,.48f,7,32,1.0f,.04f,8,7,3,.24f,4,2,.14f,0),
        CORE("Warm Ensemble",0,0,2,0,0,.35f,8,.18f,4600,.30f,1.4f,2.6f,.90f,3.6f,.38f,5,14,.82f,.08f,4,1,5,.10f,3,7,.08f,0),
        CORE("Shimmer Current",1,3,2,4,0,.54f,9,.00f,11000,.14f,1.8f,2.0f,.78f,4.6f,.32f,6,20,.98f,.00f,13,2,3,.12f,9,8,.13f,0),
        CORE("Obsidian Bed",0,1,0,5,0,.20f,3,.25f,1700,.55f,2.8f,3.2f,.96f,5.0f,.72f,4,7,.72f,.18f,6,1,1,.18f,9,10,.10f,0),
        CORE("Wide Afterglow",4,0,2,2,0,.58f,31,.02f,6100,.24f,1.2f,2.5f,.88f,4.4f,.40f,7,42,1.0f,.02f,2,1,5,.12f,9,7,.12f,0)
    },
    { // ATMOSPHERES
        CORE("Deep Orbit",0,3,2,4,0,.58f,5,.00f,3200,.16f,3.1f,3.8f,.96f,5.0f,.55f,5,12,.92f,.00f,5,1,3,.18f,1,9,.16f,0),
        CORE("Frozen Nebula",4,2,3,2,0,.72f,34,.00f,4600,.12f,2.8f,3.4f,.92f,5.0f,.42f,7,44,1.0f,.00f,8,7,5,.20f,1,8,.10f,0),
        CORE("Distant Choir",1,3,3,4,1,.55f,3,.00f,2400,.32f,3.8f,4.0f,.90f,5.0f,.62f,4,9,.78f,.00f,9,2,1,.16f,9,10,.20f,0),
        CORE("Spectral Rain",2,3,2,3,2,.42f,7,.00f,7800,.18f,1.6f,2.7f,.54f,4.8f,.34f,3,15,.70f,.05f,7,7,4,.25f,9,5,.18f,0),
        CORE("Weightless Dust",0,2,3,5,0,.50f,10,.00f,2800,.24f,2.0f,3.1f,.82f,4.2f,.68f,6,25,.94f,.02f,13,1,3,.18f,7,5,.14f,0),
        CORE("Solar Wind",4,0,2,3,0,.44f,28,.00f,5600,.20f,1.4f,2.5f,.88f,4.8f,.42f,7,40,1.0f,.08f,8,7,3,.22f,9,8,.18f,0),
        CORE("Void Lantern",1,3,3,1,0,.36f,2,.00f,1400,.50f,3.2f,4.0f,.98f,5.0f,.86f,4,8,.78f,.10f,6,1,1,.28f,9,10,.12f,0),
        CORE("Shattered Halo",3,0,1,5,2,.68f,16,.00f,9200,.22f,.06f,1.6f,.30f,3.4f,.35f,3,24,.86f,.24f,7,7,4,.28f,9,5,.18f,0),
        CORE("Endless Tide",0,2,3,2,0,.62f,14,.00f,3600,.18f,2.6f,3.6f,.94f,5.0f,.72f,7,26,1.0f,.00f,2,1,3,.16f,9,8,.16f,0),
        CORE("Afterimage",2,3,2,4,1,.48f,4,.00f,6800,.16f,2.2f,2.8f,.64f,4.8f,.44f,4,12,.84f,.00f,12,7,5,.20f,8,4,.14f,0)
    },
    { // KEYS
        CORE("FM Tines",1,3,2,4,0,.78f,1,.00f,12500,.12f,.001f,.55f,.18f,.8f,.25f,1,0,.0f,.00f,1,3,1,.18f,9,7,.10f,0),
        CORE("Soft Poly",0,2,3,2,0,.42f,4,.02f,7200,.14f,.004f,.42f,.46f,.55f,.12f,3,8,.55f,.00f,1,3,1,.12f,9,5,.08f,0),
        CORE("Digital EP",1,3,0,3,0,.36f,3,.00f,9800,.18f,.001f,.75f,.12f,.6f,.22f,1,0,.0f,.04f,4,3,6,.12f,9,7,.10f,0),
        CORE("Bell Circuit",1,3,3,4,2,.80f,0,.00f,15500,.10f,.001f,1.0f,.0f,.9f,.16f,2,6,.28f,.00f,3,3,4,.20f,9,5,.14f,0),
        CORE("Muted Clav",0,1,2,1,0,.28f,8,.00f,3600,.42f,.001f,.18f,.16f,.22f,.82f,1,2,.12f,.14f,4,3,1,.22f,9,7,.08f,0),
        CORE("Glass Keys",1,3,2,4,0,.64f,2,.00f,14200,.12f,.001f,.68f,.10f,.75f,.20f,2,5,.35f,.00f,9,3,3,.18f,9,8,.10f,0),
        CORE("Retro Organ",0,1,0,0,0,.10f,0,.00f,7800,.18f,.002f,.08f,.92f,.14f,.10f,2,2,.30f,.04f,2,1,5,.08f,9,7,.10f,0),
        CORE("Pulse Keys",3,1,0,3,0,.38f,10,.00f,6800,.34f,.001f,.30f,.34f,.28f,.42f,1,4,.15f,.12f,8,2,1,.12f,3,6,.08f,0),
        CORE("Rubber Keys",2,2,3,5,0,.55f,6,.00f,5200,.28f,.001f,.40f,.22f,.34f,.30f,1,3,.10f,.18f,7,3,5,.18f,9,7,.08f,0),
        CORE("Velocity Bloom",1,3,2,2,0,.50f,5,.00f,8200,.18f,.002f,.36f,.32f,.65f,.34f,3,12,.50f,.00f,2,3,1,.28f,9,7,.10f,0)
    },
    { // PIANOS
        CORE("Felt Room",1,3,2,4,0,.42f,1,.00f,7400,.12f,.001f,1.3f,.08f,1.2f,.24f,1,0,.0f,.00f,1,3,1,.12f,9,7,.08f,0),
        CORE("Bright Stage",1,3,0,3,0,.30f,2,.00f,14500,.10f,.001f,.82f,.12f,.9f,.18f,1,0,.0f,.02f,10,3,1,.16f,9,7,.08f,0),
        CORE("Toy Grand",1,3,2,4,0,.68f,0,.00f,16500,.12f,.001f,.46f,.03f,.48f,.12f,1,0,.0f,.00f,7,3,4,.12f,9,7,.08f,0),
        CORE("Noir Upright",0,2,3,2,0,.58f,2,.00f,5100,.30f,.001f,1.8f,.18f,1.5f,.48f,1,0,.0f,.08f,1,3,1,.12f,9,5,.12f,0),
        CORE("Glass Piano",1,3,2,4,1,.70f,0,.00f,11800,.18f,.001f,1.0f,.04f,1.7f,.20f,1,0,.0f,.00f,3,3,3,.12f,9,10,.14f,0),
        CORE("Tape Piano",1,3,0,5,0,.45f,4,.00f,6300,.28f,.002f,1.4f,.12f,1.4f,.32f,1,0,.0f,.10f,4,3,1,.10f,9,8,.08f,0),
        CORE("Studio Hammer",1,3,2,3,0,.32f,1,.00f,9800,.16f,.001f,.62f,.10f,.75f,.20f,1,0,.0f,.00f,14,3,1,.14f,9,7,.08f,0),
        CORE("Soft Cinema",1,2,3,2,0,.62f,1,.00f,5600,.18f,.002f,1.5f,.22f,2.2f,.32f,2,4,.18f,.02f,5,3,1,.14f,9,10,.12f,0),
        CORE("Electric Grand",1,3,0,4,0,.24f,0,.00f,12600,.12f,.001f,.72f,.14f,.8f,.18f,1,0,.0f,.00f,2,3,1,.10f,9,7,.08f,0),
        CORE("Distant Hall",1,3,2,4,0,.54f,2,.00f,8100,.12f,.001f,1.2f,.10f,2.8f,.20f,2,3,.20f,.00f,5,3,1,.10f,9,10,.14f,0)
    },
    { // PLUCKS
        CORE("Marble Pluck",1,3,2,4,0,.68f,1,.00f,12800,.10f,.001f,.24f,.02f,.32f,.18f,1,0,.0f,.00f,3,3,4,.12f,9,8,.10f,0),
        CORE("Rubber String",0,2,1,1,0,.18f,6,.00f,4400,.28f,.001f,.34f,.08f,.3f,.44f,1,3,.12f,.12f,4,3,1,.18f,9,7,.08f,0),
        CORE("Crystal Drop",1,3,3,4,1,.76f,0,.00f,15500,.12f,.001f,.48f,.02f,.9f,.14f,2,5,.35f,.00f,13,3,3,.20f,9,8,.14f,0),
        CORE("Muted Nylon",0,2,3,2,0,.38f,3,.02f,5300,.16f,.001f,.52f,.10f,.52f,.25f,1,0,.0f,.02f,1,3,1,.14f,9,7,.08f,0),
        CORE("FM Droplet",1,3,2,4,0,.88f,0,.00f,13800,.12f,.001f,.66f,.00f,.65f,.18f,1,0,.0f,.00f,7,3,4,.16f,9,5,.12f,0),
        CORE("Pulse Pick",3,1,0,3,0,.32f,10,.00f,8300,.34f,.001f,.22f,.06f,.34f,.45f,1,2,.10f,.14f,8,2,1,.14f,3,6,.10f,1),
        CORE("Wood Circuit",0,2,1,1,0,.28f,5,.00f,3100,.44f,.001f,.25f,.04f,.34f,.66f,1,3,.08f,.20f,4,3,6,.18f,9,7,.10f,0),
        CORE("Prism Mallet",1,3,2,4,2,.64f,0,.00f,16000,.10f,.001f,.82f,.00f,.78f,.16f,1,0,.0f,.00f,9,3,3,.16f,9,10,.12f,0),
        CORE("Echo Harp",0,2,3,2,0,.58f,13,.00f,7100,.18f,.002f,.66f,.12f,.95f,.24f,3,12,.72f,.00f,3,1,5,.16f,6,9,.18f,1),
        CORE("Shortwave",2,3,1,5,2,.42f,5,.00f,9800,.24f,.001f,.30f,.04f,.45f,.22f,1,2,.08f,.16f,7,7,4,.20f,9,5,.14f,2)
    },
    { // BASS
        CORE("Sub Anchor",0,3,0,0,0,.05f,0,.72f,780.0f,.20f,.001f,.28f,.88f,.16f,.24f,1,0,.0f,.04f,1,3,1,.08f,6,7,.08f,0),
        CORE("Reese Core",0,0,1,5,0,.28f,24,.40f,1300,.42f,.006f,.42f,.78f,.28f,.32f,2,18,.22f,.26f,4,1,1,.16f,3,5,.12f,0),
        CORE("Acid Wire",0,1,0,3,0,.14f,6,.32f,1150,.82f,.001f,.22f,.64f,.16f,.92f,1,0,.0f,.55f,14,1,1,.32f,3,6,.14f,0),
        CORE("Rubber Mono",2,2,3,5,0,.56f,2,.20f,1800,.38f,.001f,.34f,.54f,.20f,.38f,1,0,.0f,.22f,7,3,7,.12f,6,2,.10f,0),
        CORE("FM Sub",1,3,0,4,0,.82f,0,.48f,950,.24f,.001f,.62f,.42f,.22f,.26f,1,0,.0f,.08f,1,3,1,.12f,9,7,.08f,0),
        CORE("Folded Bass",0,0,1,5,0,.42f,12,.38f,2400,.68f,.001f,.30f,.62f,.20f,.78f,1,0,.0f,.62f,6,1,1,.26f,3,6,.10f,0),
        CORE("Wide Reese",4,0,2,3,0,.48f,30,.32f,1700,.32f,.008f,.52f,.72f,.42f,.34f,2,28,.46f,.22f,4,1,5,.12f,9,7,.10f,0),
        CORE("Square Bite",0,1,0,1,0,.08f,0,.52f,680,.70f,.001f,.20f,.74f,.12f,.86f,1,0,.0f,.42f,14,3,1,.28f,6,7,.08f,0),
        CORE("Deep Pulse",3,1,0,3,0,.20f,8,.58f,1320,.46f,.001f,.44f,.86f,.18f,.48f,1,3,.12f,.28f,8,2,1,.16f,3,6,.10f,0),
        CORE("Sequence Bass",0,0,1,5,0,.24f,4,.44f,1500,.72f,.001f,.28f,.70f,.18f,.90f,1,0,.0f,.48f,6,1,1,.22f,3,7,.12f,2)
    },
    { // SYNTHS
        CORE("Supersaw 90",4,0,2,3,0,.30f,26,.04f,9800,.20f,.004f,.42f,.82f,.40f,.26f,7,34,1.0f,.10f,14,1,5,.16f,9,7,.12f,0),
        CORE("Hard Sync Stack",3,0,1,3,0,.42f,18,.00f,7600,.50f,.002f,.30f,.68f,.30f,.62f,5,22,.86f,.18f,5,1,1,.20f,3,6,.12f,0),
        CORE("FM Engine",1,3,0,4,0,.88f,0,.00f,12400,.20f,.001f,.86f,.34f,.82f,.20f,2,4,.24f,.00f,9,3,3,.18f,9,10,.14f,0),
        CORE("Ring Machine",2,1,3,5,2,.54f,4,.00f,9200,.28f,.003f,.52f,.48f,.74f,.28f,3,15,.66f,.20f,12,7,4,.25f,9,5,.18f,0),
        CORE("PWM Matrix",0,1,0,1,0,.28f,12,.00f,5800,.30f,.008f,.44f,.76f,.32f,.34f,4,18,.82f,.06f,2,1,2,.22f,7,5,.14f,0),
        CORE("Digital Stack",1,3,2,3,0,.72f,5,.00f,14200,.14f,.001f,.72f,.18f,.88f,.20f,4,13,.72f,.08f,7,7,3,.20f,9,8,.12f,0),
        CORE("Analog Poly",0,0,2,0,0,.34f,9,.06f,6400,.24f,.012f,.62f,.62f,.60f,.28f,6,16,.90f,.08f,2,1,5,.12f,3,7,.10f,0),
        CORE("Wide Detune",4,0,3,2,0,.58f,38,.00f,8200,.18f,.02f,.78f,.84f,.92f,.20f,7,48,1.0f,.04f,2,1,5,.10f,9,7,.12f,0),
        CORE("Hybrid Motion",3,0,2,5,0,.62f,15,.04f,5100,.38f,.18f,.88f,.66f,1.4f,.58f,5,26,.90f,.24f,8,7,3,.28f,1,5,.12f,0),
        CORE("Arp Engine",1,3,0,4,0,.62f,1,.00f,11200,.22f,.001f,.40f,.16f,.32f,.24f,2,8,.42f,.08f,3,3,4,.20f,9,7,.10f,1)
    },
    { // SOUND FX
        CORE("Reverse Swell",0,2,3,2,0,.62f,2,.00f,4500,.18f,2.8f,1.2f,.72f,3.8f,.40f,5,16,.88f,.04f,5,1,3,.32f,9,10,.20f,0),
        CORE("Laser Fall",3,0,1,5,2,.82f,4,.00f,13800,.30f,.001f,.42f,.02f,1.8f,.26f,1,0,.0f,.22f,7,7,4,.28f,9,5,.20f,2),
        CORE("Glitch Spark",2,3,1,5,2,.54f,0,.00f,15000,.12f,.001f,.18f,.00f,.42f,.18f,1,0,.0f,.48f,7,7,4,.35f,9,5,.20f,2),
        CORE("Impact Bloom",4,0,1,3,0,.46f,25,.12f,3800,.52f,.001f,.90f,.00f,2.2f,.72f,5,32,.96f,.54f,6,3,1,.30f,9,10,.20f,0),
        CORE("Metal Orbit",1,3,0,4,1,.90f,0,.00f,11200,.44f,.002f,2.4f,.08f,4.4f,.50f,2,4,.48f,.20f,12,7,4,.32f,9,5,.24f,0),
        CORE("Radio Ghost",2,3,2,3,2,.68f,8,.00f,7200,.58f,.08f,.62f,.26f,2.8f,.64f,3,20,.72f,.36f,7,7,4,.34f,9,1,.20f,0),
        CORE("Voltage Burst",3,1,0,5,0,.26f,22,.00f,9800,.72f,.001f,.14f,.00f,.75f,.86f,2,18,.54f,.72f,14,3,1,.28f,9,5,.18f,2),
        CORE("Downshift",0,0,2,5,0,.36f,10,.22f,2500,.60f,.001f,1.8f,.08f,2.4f,.84f,2,12,.42f,.48f,8,8,4,.20f,7,5,.18f,0),
        CORE("Riser Field",4,0,2,3,0,.68f,34,.00f,8800,.18f,2.2f,2.2f,.96f,4.0f,.58f,7,42,1.0f,.24f,8,7,1,.26f,8,4,.16f,0),
        CORE("Signal Tear",2,1,3,5,1,.70f,5,.00f,9200,.48f,.001f,.34f,.04f,1.4f,.60f,2,14,.62f,.40f,7,7,4,.36f,9,5,.18f,2)
    },
    { // TEXTURES
        CORE("Wavetable Drift",0,2,3,2,0,.72f,7,.00f,4200,.18f,2.6f,3.8f,.88f,4.8f,.62f,6,24,.96f,.00f,9,1,3,.26f,7,5,.12f,0),
        CORE("Frozen Glass",1,3,2,4,0,.82f,1,.00f,14500,.12f,1.8f,2.8f,.82f,4.4f,.32f,5,12,.90f,.00f,5,2,3,.18f,9,10,.20f,0),
        CORE("Soft Static",2,3,1,5,2,.42f,0,.00f,11000,.22f,1.2f,2.4f,.44f,3.8f,.46f,3,8,.72f,.18f,7,7,4,.28f,9,5,.18f,0),
        CORE("Slow Circuit",3,0,1,3,0,.50f,15,.00f,3900,.52f,2.2f,3.0f,.86f,4.6f,.76f,5,26,.92f,.16f,8,1,1,.24f,7,3,.14f,0),
        CORE("Shimmer Field",4,0,2,4,0,.64f,30,.00f,9800,.14f,2.8f,3.4f,.92f,5.0f,.42f,7,42,1.0f,.00f,2,1,5,.18f,9,8,.14f,0),
        CORE("Hollow Wind",0,3,2,2,1,.58f,4,.00f,2400,.24f,3.0f,3.5f,.72f,4.8f,.68f,4,10,.82f,.02f,13,1,1,.24f,9,10,.16f,0),
        CORE("Dark Matter",2,1,3,5,0,.38f,4,.24f,1200,.64f,2.2f,3.2f,.92f,5.0f,.90f,5,12,.82f,.28f,6,1,1,.24f,9,10,.14f,0),
        CORE("Velvet Dust",0,2,3,1,0,.46f,12,.00f,5100,.20f,1.8f,2.8f,.86f,4.2f,.36f,6,22,.94f,.00f,2,1,5,.16f,9,7,.14f,0),
        CORE("Fractal Tide",1,3,2,3,1,.76f,2,.00f,7800,.28f,2.5f,3.6f,.82f,4.8f,.54f,5,18,.92f,.08f,7,7,3,.30f,9,5,.18f,0),
        CORE("Memory Cloud",4,2,3,2,0,.56f,22,.00f,6200,.12f,3.2f,3.8f,.94f,5.0f,.44f,7,36,1.0f,.00f,5,1,3,.20f,9,10,.20f,0)
    }
};
#undef CORE

inline const Core& coreFor(int id) noexcept {
    const int safeId = juce::jlimit(1, 500, id);
    return cores[(size_t)((safeId - 1) / 50)][(size_t)(((safeId - 1) % 50) / 5)];
}

inline juce::String name(int id) {
    if (id < 1 || id > 500) return {};
    static constexpr const char* variants[5] = {
        "CORE", "WIDE", "DARK", "MOTION", "LIVE"
    };
    const int variant = (id - 1) % 5;
    return juce::String(coreFor(id).name) + " — " + variants[variant];
}

inline juce::String category(int id) {
    if (id < 1 || id > 500) return "ALL SOUNDS";
    const auto& c = coreFor(id);
    if (c.seqProfile != 0) return "SEQUENCES";
    static constexpr const char* families[10] = {
        "LEADS", "PADS", "ATMOSPHERES", "KEYS", "PIANOS",
        "PLUCKS", "BASS", "SYNTHS", "SOUND FX", "TEXTURES"
    };
    return families[(size_t)((id - 1) / 50)];
}

inline void apply(AuroraProcessor::APVTS& params, int id) {
    if (id < 1 || id > 500) return;
    const Core& core = coreFor(id);
    const int family = (id - 1) / 50;
    const int archetype = ((id - 1) % 50) / 5;
    const int variant = (id - 1) % 5;

    auto set = [&params](const juce::String& key, float value) {
        if (auto* parameter = params.getParameter(key))
            parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
    };
    auto clamp01 = [](float value) { return juce::jlimit(0.0f, 1.0f, value); };
    auto hasFx = [](int profile, int effect) {
        static constexpr int chains[15][6] = {
            {0,0,0,0,0,0}, {1,5,0,0,0,0}, {6,5,0,0,0,0},
            {4,5,0,0,0,0}, {2,6,5,0,0,0}, {3,4,5,0,0,0},
            {7,5,0,0,0,0}, {9,12,14,5,0,0}, {11,3,4,5,0,0},
            {13,6,5,0,0,0}, {8,5,0,0,0,0}, {10,6,5,0,0,0},
            {12,3,5,0,0,0}, {14,4,5,0,0,0}, {1,2,4,5,0,0}
        };
        for (int slot = 0; slot < 6; ++slot)
            if (chains[juce::jlimit(0,14,profile)][slot] == effect) return true;
        return false;
    };

    float mix = core.mix, detune = core.detune, sub = core.sub;
    float cutoff = core.cutoff, resonance = core.resonance;
    float attack = core.attack, decay = core.decay, sustain = core.sustain, release = core.release;
    float filterAmount = core.filterAmount;
    float voices = (float)core.unison, uniDetune = core.uniDetune, spread = core.spread;
    float drive = core.drive;
    int fxProfile = core.fxProfile;
    int mod1Source = core.mod1Source, mod1Target = core.mod1Target;
    float mod1Amount = core.mod1Amount;
    int mod2Source = core.mod2Source, mod2Target = core.mod2Target;
    float mod2Amount = core.mod2Amount;

    switch (variant) {
        case 1: // Wide: more detune and stereo, never turn a mono bass into a wash.
            voices = family == 6 ? juce::jmin(2.0f, voices + 1.0f) : juce::jmin(7.0f, voices + 1.0f);
            detune += family == 6 ? 1.5f : 4.0f;
            uniDetune = juce::jmin(50.0f, uniDetune + 8.0f);
            spread = family == 6 ? juce::jmin(0.35f, spread + 0.12f) : juce::jmin(1.0f, spread + 0.20f);
            mix = clamp01(mix + 0.06f);
            break;
        case 2: // Dark: lower filter, fuller fundamental, slower release.
            cutoff *= 0.62f;
            resonance = juce::jmin(0.92f, resonance + 0.05f);
            sub = clamp01(sub + 0.08f);
            filterAmount = juce::jmin(1.0f, filterAmount + 0.12f);
            release = juce::jmin(5.0f, release * 1.12f);
            break;
        case 3: // Motion: purposeful modulation and movement, not random knob noise.
            cutoff *= 1.08f;
            mod1Source = mod1Source == 0 ? 1 : mod1Source;
            mod1Target = mod1Target == 0 ? (family == 1 || family == 9 ? 3 : 1) : mod1Target;
            mod1Amount = juce::jlimit(-0.85f, 0.85f,
                (mod1Amount == 0.0f ? 0.18f : mod1Amount * 1.30f));
            mod2Source = mod2Source == 0 ? 7 : mod2Source;
            mod2Target = mod2Target == 0 ? 5 : mod2Target;
            mod2Amount = mod2Amount == 0.0f ? 0.16f : juce::jlimit(-0.8f,0.8f,mod2Amount);
            break;
        case 4: // Live: firmer level, brighter projection and a little more space.
            cutoff = juce::jmin(17500.0f, cutoff * 1.14f);
            attack = juce::jmax(0.001f, attack * 0.82f);
            drive = juce::jmin(0.72f, drive + 0.05f);
            break;
        default: break;
    }

    voices = juce::jlimit(1.0f, 7.0f, voices);
    spread = clamp01(spread);
    mix = clamp01(mix);
    sub = clamp01(sub);

    set("wave1",(float)core.wave1);
    set("wave2",(float)core.wave2);
    set("wtTable",(float)core.table);
    set("oscMode",(float)core.mode);
    set("filterType",(float)core.filter);
    set("mix",mix);
    set("detune",detune);
    set("sub",sub);
    set("cutoff",cutoff);
    set("res",resonance);
    set("attack",attack);
    set("decay",decay);
    set("sustain",sustain);
    set("release",release);
    set("filterEnvAmount",filterAmount);
    const float filterAttack = juce::jlimit(0.001f,4.0f,
        (attack < 0.08f) ? juce::jmax(0.004f, attack * 1.8f) : attack * 0.58f);
    const float filterDecay = juce::jlimit(0.01f,5.0f,decay * (family == 6 ? 0.42f : 0.72f));
    const float filterSustain = juce::jlimit(0.0f,1.0f,
        sustain * (family == 1 || family == 2 || family == 9 ? 0.78f : 0.36f));
    const float filterRelease = juce::jlimit(0.01f,5.0f,release * 0.78f);
    set("filterAttack",filterAttack);
    set("filterDecay",filterDecay);
    set("filterSustain",filterSustain);
    set("filterRelease",filterRelease);
    set("fmAmount",core.mode == 1 ? (3.0f + (float)(archetype % 5) * 1.25f) : 2.5f);
    set("syncRatio",core.mode == 3 ? (2.0f + (float)(archetype % 5)) : 2.0f + (float)(archetype % 3));
    set("master",juce::jlimit(0.56f,0.82f,0.72f - (family == 8 ? 0.06f : 0.0f)
        + (float)((archetype + variant) % 4) * 0.018f));
    set("drive",drive);
    set("width",juce::jlimit(0.25f,1.0f,0.66f + spread * 0.30f));
    set("unison",voices);
    set("uniDetune",uniDetune);
    set("uniSpread",spread);
    const float position = clamp01(0.10f + (float)((archetype * 3 + variant * 2 + family) % 9) * 0.095f);
    set("wtPosition",position);

    // Choose an intentional variant chain before writing every FX slot.
    if (variant == 3 && fxProfile == 0) fxProfile = 8;
    if (variant == 4 && fxProfile == 0) fxProfile = 1;

    // Each family has a deliberate effects topology; six slots are fully rewritten per patch.
    static constexpr int chains[15][6] = {
        {0,0,0,0,0,0}, {1,5,0,0,0,0}, {6,5,0,0,0,0},
        {4,5,0,0,0,0}, {2,6,5,0,0,0}, {3,4,5,0,0,0},
        {7,5,0,0,0,0}, {9,12,14,5,0,0}, {11,3,4,5,0,0},
        {13,6,5,0,0,0}, {8,5,0,0,0,0}, {10,6,5,0,0,0},
        {12,3,5,0,0,0}, {14,4,5,0,0,0}, {1,2,4,5,0,0}
    };
    for (int slot=0; slot<6; ++slot)
        set("fxSlot" + juce::String(slot+1),(float)chains[juce::jlimit(0,14,fxProfile)][slot]);
    const bool hasCompressor = hasFx(fxProfile,1);
    const bool hasSaturation = hasFx(fxProfile,2);
    const bool hasChorus = hasFx(fxProfile,6);
    const bool hasPing = hasFx(fxProfile,4);
    const bool hasReverb = hasFx(fxProfile,5);
    const float spaceScale = variant == 2 ? 0.72f : (variant == 4 ? 1.12f : 1.0f);
    set("fxEq",0.20f + (float)((archetype + family) % 4) * 0.07f);
    set("fxLow",family == 6 ? 1.5f : (family == 8 ? -1.0f : -0.5f + (float)(archetype%3)*0.5f));
    set("fxMid",family == 6 ? -0.5f : -1.0f + (float)((archetype+variant)%5)*0.5f);
    set("fxHigh",variant == 2 ? -2.5f : 0.5f + (float)((archetype*2+variant)%5)*0.65f);
    set("fxDist",hasSaturation ? juce::jlimit(0.08f,0.76f,drive + 0.14f + (float)(archetype%3)*0.04f) : drive * 0.35f);
    set("fxChorus",hasChorus ? juce::jlimit(0.12f,0.72f,(family==1 || family==2 || family==9 ? 0.44f : 0.25f)
        + (float)(archetype%3)*0.07f) : 0.0f);
    set("fxFlange",hasFx(fxProfile,3) ? 0.28f + 0.06f*(float)(archetype%3) : 0.0f);
    set("fxDelay",hasPing ? juce::jlimit(0.16f,0.72f,0.32f + (float)(archetype%4)*0.09f) : 0.0f);
    set("fxReverb",hasReverb ? juce::jlimit(0.18f,0.78f,(family==1 || family==2 || family==9 ? 0.55f : 0.30f)*spaceScale) : 0.0f);

    // Compressor, saturation, modulation and every expanded FX parameter are deterministic.
    set("fxCompEnabled",hasCompressor ? 1.0f : 0.0f);
    set("fxCompThreshold",family==6 ? -22.0f : -18.0f);
    set("fxCompRatio",family==6 ? 5.5f : 3.0f + (float)(archetype%3));
    set("fxCompAttack",family==6 ? 4.0f : 12.0f);
    set("fxCompRelease",family==6 ? 95.0f : 145.0f);
    set("fxCompMakeup",family==6 ? 2.0f : 0.8f);
    set("fxCompMix",0.82f);
    set("fxSatTone",-0.25f + (float)((archetype+variant)%5)*0.10f);
    set("fxSatMix",hasSaturation ? 0.82f : 0.0f);
    set("fxPhaserRate",0.12f + (float)((archetype+variant)%6)*0.16f);
    set("fxPhaserFeedback",0.12f + (float)(archetype%5)*0.11f);
    set("fxPingTime",180.0f + (float)((archetype*3+variant)%7)*72.0f);
    set("fxPingFeedback",0.28f + (float)(archetype%5)*0.085f);
    set("fxPingTone",0.42f + (float)((archetype+variant)%5)*0.10f);
    set("fxReverbRoom",juce::jlimit(0.18f,0.98f,0.38f + (family==1 || family==2 || family==9 ? 0.32f : 0.0f)
        + (float)(archetype%4)*0.06f));
    set("fxReverbDamping",0.32f + (float)((archetype+family)%5)*0.10f);
    set("fxReverbWidth",juce::jlimit(0.45f,1.0f,spread));
    set("fxLimiterCeiling",-1.0f);
    set("fxLimiterInput",family==8 ? 3.0f : 0.0f);
    set("fxLimiterMix",0.90f);
    set("fxLimiterRelease",75.0f + (float)(archetype%4)*30.0f);
    set("fxMbLow",family==6 ? 3.5f : 1.8f);
    set("fxMbMid",2.2f + (float)(archetype%4)*0.65f);
    set("fxMbHigh",1.4f + (float)((archetype+2)%4)*0.7f);
    set("fxMbMix",0.68f);
    set("fxBitDepth",variant==3 ? 6.0f + (float)(archetype%4)*1.5f : 10.0f);
    set("fxBitRate",variant==3 ? 5.0f + (float)(archetype%4)*2.0f : 2.0f);
    set("fxBitMix",hasFx(fxProfile,9) ? 0.48f : 0.0f);
    set("fxStereoWidth",juce::jlimit(0.5f,1.8f,0.88f + spread*0.72f));
    set("fxStereoPan",family==8 ? (variant==2 ? -0.18f : 0.12f) : 0.0f);
    set("fxStereoMix",0.82f);
    set("fxTremRate",0.35f + (float)((archetype+family)%7)*0.55f);
    set("fxTremDepth",hasFx(fxProfile,11) ? 0.28f + (float)(archetype%4)*0.12f : 0.0f);
    set("fxTremStereo",45.0f + (float)(archetype%4)*30.0f);
    set("fxTremMix",0.82f);
    set("fxRingFreq",70.0f + (float)((archetype*83+family*47)%900));
    set("fxRingMix",hasFx(fxProfile,12) ? 0.28f + (float)(archetype%4)*0.13f : 0.0f);
    set("fxRingStereo",90.0f);
    set("fxFilterCutoff",family==6 ? 1400.0f : 3200.0f + (float)(archetype%6)*1800.0f);
    set("fxFilterRes",0.18f + (float)(archetype%5)*0.12f);
    set("fxFilterType",(float)((archetype+family)%3));
    set("fxFilterMix",hasFx(fxProfile,13) ? 0.62f : 0.0f);
    set("fxFlangerRate",0.12f + (float)((archetype+variant)%6)*0.18f);
    set("fxFlangerDepth",0.24f + (float)(archetype%4)*0.14f);
    set("fxFlangerFeedback",-0.12f + (float)(archetype%5)*0.14f);
    set("fxFlangerTime",1.2f + (float)(archetype%5)*1.1f);
    set("fxFlangerMix",hasFx(fxProfile,14) ? 0.42f : 0.0f);
    set("fxChorusRate",0.10f + (float)((archetype+variant)%6)*0.14f);
    set("fxChorusDepth",0.30f + (float)(archetype%4)*0.16f);
    set("fxChorusWidth",juce::jlimit(0.2f,1.0f,spread));

    // Three distinct LFO lanes and up to four active modulation routes per patch.
    set("lfo1Rate",0.08f + (float)((archetype*3+variant)%11)*0.24f);
    set("lfo2Rate",0.035f + (float)((archetype+family*2)%9)*0.13f);
    set("lfo3Rate",0.12f + (float)((archetype*2+variant)%8)*0.42f);
    set("lfo1Shape",(float)((archetype+family)%5));
    set("lfo2Shape",(float)((archetype+2*family+1)%5));
    set("lfo3Shape",(float)((archetype+variant+3)%5));
    int routeSources[4] = { mod1Source, mod2Source, 0, 0 };
    int routeTargets[4] = { mod1Target, mod2Target, 0, 0 };
    float routeAmounts[4] = { mod1Amount, mod2Amount, 0.0f, 0.0f };
    if (variant == 3) {
        routeSources[2] = 7;
        routeTargets[2] = (family==1 || family==2 || family==9) ? 3 : 5;
        routeAmounts[2] = 0.18f;
        routeSources[3] = 9;
        routeTargets[3] = family==6 ? 7 : 1;
        routeAmounts[3] = family==6 ? 0.12f : 0.16f;
    } else if ((archetype + family) % 4 == 0) {
        routeSources[2] = 3;
        routeTargets[2] = 6;
        routeAmounts[2] = 0.14f;
    }
    for (int route=0; route<4; ++route) {
        const auto suffix = juce::String(route+1);
        set("mod"+suffix+"Source",(float)routeSources[route]);
        set("mod"+suffix+"Target",(float)routeTargets[route]);
        set("mod"+suffix+"Amount",routeAmounts[route]);
    }

    // Sequencer is deliberately used by selected arps and rhythmic patches.
    const int seqMode = core.seqProfile == 1 ? 1 : (core.seqProfile == 2 ? 2 : 0);
    set("seqMode",(float)seqMode);
    set("arpStyle",(float)((archetype + family + variant) % 4));
    set("seqRate",(float)((archetype + family) % 5));
    set("seqOctaves",(float)(1 + (archetype % 3)));
    set("seqGate",seqMode==0 ? 82.0f : (seqMode==1 ? 74.0f : 84.0f));
    set("seqSwing",seqMode==0 ? 0.0f : 8.0f + (float)((archetype+variant)%5)*6.0f);
    set("seqDirection",(float)((archetype + variant) % 4));
    const int lengthChoice = seqMode==2 ? 1 + (archetype%3) : 2;
    set("seqLength",(float)lengthChoice);
    set("seqPage",0.0f);
    set("seqPatternSlot",0.0f);
    static constexpr int patterns[5][16] = {
        {0,7,12,7,3,10,12,10,0,7,15,12,3,10,7,12},
        {0,0,7,0,12,7,3,0,0,7,12,15,7,3,10,12},
        {0,12,7,15,3,10,5,12,0,7,17,12,3,10,5,15},
        {0,5,12,7,-2,10,14,7,0,5,12,17,3,10,14,7},
        {0,7,3,10,12,5,15,7,0,10,5,12,3,15,7,12}
    };
    const int patternIndex = (archetype + family + variant) % 5;
    const int patternLength = lengthChoice==1 ? 8 : (lengthChoice==2 ? 16 : 32);
    for (int step=0; step<32; ++step) {
        const int stepInPattern = step % 16;
        const bool enabled = seqMode==2
            ? (step < patternLength && !((step + archetype) % (archetype%3 + 7) == 5))
            : true;
        set("stepOn"+juce::String(step),enabled ? 1.0f : 0.0f);
        set("stepPitch"+juce::String(step),(float)juce::jlimit(-12,12,patterns[patternIndex][stepInPattern]));
        set("stepVelocity"+juce::String(step),(float)(82 + ((step*13 + archetype*7 + variant*5) % 46)));
        set("stepGate"+juce::String(step),(float)(60 + ((step*9 + archetype*11) % 36)));
        set("stepAccent"+juce::String(step),((step + archetype) % 4 == 0) ? 1.0f : 0.0f);
    }
}
} // namespace FactoryPresets
