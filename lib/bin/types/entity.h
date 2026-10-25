#pragma once

#include "../utils/routine.h"

#include "../utils/global.h"
#include "../utils/log.h"

#include "modifier.h"
#include "texture.h"
#include <functional>
#include <memory>
#include <unordered_map>

#include "../src/version.h"
#include "../src/objects.cpp"
#include "../utils/serialization_def.cpp"

namespace Gmeng {
    /// Type decl
    class EntityBase;

    struct Entity_Interaction {
      public:
        enum Interaction_Type {
            LMB_CLICK, RMB_CLICK,
            MMB_CLICK, KEYPRESS,
            PROXIMITY, SCRIPT,
            UNKNOWN_INTERACTION
        };

        Interaction_Type type;
        std::shared_ptr<EntityBase> interacted_by;
    };
    /// type decl
    class Level;

    /// type decl
    class EventLoop;

    class EntityBase {
      protected:
        // Entity derivatory class id assignment
        // this is increasingly added each time
        // a new class derives from 'Entity'.
        // with the Entity class.
        static int id_counter;

        // Registers a new entity derivatory.
        // Required for serialization, otherwise
        // all entities would be saved as a different type.
        template <typename Derived>
        static void register_derived_class(int id) {
            get_derived_factory()[id] = []() { return std::make_unique<Derived>(); };
        };

      public:
        /// DO NOT TOUCH, USE OR RUN THIS METHOD IF YOU
        /// ARE NOT AWARE OF WHAT IT WILL DO. YOU WILL BREAK
        /// YOUR ENTITY ID SYSTEM AND FUCK YOUR LEVEL UP.
        static int incr_id(int value = -1) {
            if (value == -1) return id_counter++;
            else return id_counter = value+1;
        };
        /// decides whether an entity will be rendered / utilized or not.
        bool active = true;

        /// Returns the derivatory factory containing the map
        /// for the function that constructs the current Entity type.
        ///
        /// Used for serialization.
        static std::unordered_map<int, std::function<std::unique_ptr<EntityBase>()>>& get_derived_factory() {
            static std::unordered_map<int, std::function<std::unique_ptr<EntityBase>()>> factory_map;
            return factory_map;
        }

        // well, brave calling this a "sprite".
        // Texture of the entity.
        texture sprite;
        // position of the Entity.
        // The position is not CENTRAL.
        // The 'position' value returns the position of the 1st unit of the
        // entity.
        // For example:
        //
        // 'o' for map, 'x' for entity units
        //
        // oooo
        // ooXx
        // ooxx
        //
        // the capitalised 'X' is the position value.
        // So in this case it would be (x=2,y=1)
        Renderer::drawpoint position = { 0, 0 };
        // random id, to differenciate entities
        // does not have to be user-set unless
        // the entity is being duplicated.
        //
        // constructor() initializes with g_mkid().
        // see gmeng.h:g_mkid() for info.
        unsigned int entity_id = g_mkid();
        // interaction proximity,
        // the amount of units a player should be
        // in proximity to the entity for the
        // entity->interact() function to be called.
        //
        // Defaults to 5
        unsigned int interaction_proximity = 5;

        /// cache status of the entity.
        /// Can be used to send client updates in multiplayer scenarios.
        bool cached = false;

        /// modifiers of the entity.
        ModifierList modifiers;

        /// Routines of the entity. They are executed according
        /// to a specified period with its handler being a lua
        /// function `entity_handler`.
        RoutineList routines;

        GMENG_INIT_TYPE( active, sprite, position, entity_id, routines,
                         interaction_proximity, cached, modifiers );

        /// Entity creation
        EntityBase() = default;

        /// Entity from a texture and position.
        EntityBase(texture model_, Renderer::drawpoint pos_) :
            sprite(model_), position(pos_) {
                if (!this->modifiers.has_value("z_index"))
                    this->modifiers.add_value("z_index", 0);
            };

        /// Default deconstructor
        virtual ~EntityBase() = default;

        /// Returns the serialization id of the
        /// derived Entity class.
        virtual int get_serialization_id() const = 0;

        /// default serialization code for base entity objects.
        inline void default_serialize( std::ostream& out ) const {
            out.write(reinterpret_cast<const char*>(&entity_id), sizeof(entity_id));
            out.write(reinterpret_cast<const char*>(&interaction_proximity), sizeof(interaction_proximity) );

            // cant read-write drawpoints yet, so done manually
            out.write(reinterpret_cast<const char*>(&position.x), sizeof(position.x));
            out.write(reinterpret_cast<const char*>(&position.y), sizeof(position.y));

            serialize_modifier_list( this->modifiers, out );

            serialize_texture(sprite, out);
        };

        /// default deserialization code for base entity objects.
        inline void default_deserialize( std::istream& in ) {
            in.read(reinterpret_cast<char*>(&entity_id), sizeof(entity_id));
            in.read(reinterpret_cast<char*>(&interaction_proximity), sizeof(interaction_proximity) );

            // cant read-write drawpoints yet, so done manually
            in.read(reinterpret_cast<char*>(&position.x), sizeof(position.x));
            in.read(reinterpret_cast<char*>(&position.y), sizeof(position.y));

            deserialize_modifier_list(this->modifiers, in);

            if ( !modifiers.has_value("z_index") )
                modifiers.add_value("z_index", 0);

            deserialize_texture(sprite, in);
        };
        /// serialization function.
        /// Classes can override this function to add different serialization code.
        virtual void serialize(std::ostream& out) const {
            this->default_serialize( out );
        };
        /// deserialization function.
        /// Classes can override this function to add diferent deserialization code.
        virtual void deserialize(std::istream& in) {
            this->default_deserialize( in );
        };

        /// Method to be called when requesting a texture
        /// for the entity.
        ///
        /// This is used because the Entity::sprite texture
        /// may not be finalized.
        ///
        /// The draw function allows for changes in the sprite
        /// with the ability to take into account other variables
        /// like the EventLoop states.
        ///
        /// e.g. zombies eyes turn red when the player is
        /// within 5 units of proximity
        virtual texture& draw( Renderer_Type type = CONSOLE ) {
            // default behaviour for drawing the entity.
            // can be extended with subclasses
            return this->sprite;
        };

        /// Internal execution function for this->interact()
        /// for multiplayer-mode caching check
        void _interact( Entity_Interaction interaction, Level* lvl ) {
            this->cached = false;
            this->interact( interaction, lvl );
            gm_log("an interaction occured finally");
        };

        /// Internal execution function for this->periodic()
        /// for multiplayer-mode caching check
        void _periodic( EventLoop* ev ) {
            this->cached = false;
            this->periodic( ev );
        };

        /// Internal execution function for this->animate()
        /// for multiplayer-mode caching check
        void _animate( EventLoop* ev, int FPS = 60) {
            this->cached = false;
            this->animate( ev, FPS );
        };

        /// Method to be called when any interaction with
        /// an entity takes place.
        ///
        /// Interactions have proximity, using the
        /// Entity::interaction_proximity value.
        virtual void interact( Entity_Interaction, Level* ) { /* no default behaviour */ };

        /// Method to be called periodically at every FIXED_UPDATE event
        /// overriding this method allows you to give functionality & AI to entities.
        ///
        /// Examples of this can be following the player,
        /// checking for proximity with other entities, etc.
        virtual void periodic( EventLoop* ev = NULL ) { /* no default behaviour */ };

        /// This method is called periodically every 250ms for all entities.
        /// It is to provide animation capabilities with a stable interval.
        ///
        /// This way, you do not have to rely on the periodic() method to animate
        /// the texture of an entity.
        ///
        /// NOT REQUIRED! Animated sprites are optional.
        virtual void animate( EventLoop* ev = NULL, int FPS = 60 ) { /* no default behaviour */ };
    };
    /// sets off the derivatory id count from 0.
    inline int EntityBase::id_counter = 0;

    /// Virtual class for Extendable Entity
    /// classes.
    ///
    /// Derive this class with:
    /// ```
    /// class MyEntityType : public Entity<MyEntityType> {
    ///
    /// };
    /// ```
    /// and add behaviour to interact(), draw(), animate()
    /// and etc to your new entity type.
    template <typename Derived>
    class Entity : public EntityBase {
      public:
        /// Derived entity ID, used to distingish entity types.
        /// Entity<Derived>::id will be uniquely incremented from
        /// any other Entity<Derived2>. But, Entity<Derived>::id will
        /// be equal to another Entity<Derived>::id.
        ///
        /// To distinguish Entities from each other (not entity types),
        /// use Entity<Derived>.entity_id. Uniquely generated with g_mkid().
        /// see gmeng.h:g_mkid() for information on ID generation.
        static const int id;

        Entity() {
            // register the new class (that extends this) to the derived class factory
            Entity::register_derived_class<Derived>(id);
        };
        /// returns the serialization id (so the derived class id)
        /// of the derivatory class that extends Entity : EntityBase.
        int get_serialization_id() const override {
            return id;
        };
    };

    /// Set derivatory class ID.
    template <typename Derived>
    const int Entity<Derived>::id = EntityBase::id_counter++;

    /// Dictionary of all registered entities visible to the engine.
    /// Returns the name of the entity type from the entity serialization id.
    inline std::map<int, std::string> entity_dictionary;

    /// returns the name of an entity type from its serialization id.
    static const std::string get_entity_name( const int serialization_id ) {
        if ( entity_dictionary.contains( serialization_id ) )
            return entity_dictionary.at( serialization_id );
        else return "UNKNOWN_ENTITY" + v_str(serialization_id);
    };

    template<typename Derived>
    class DerivedEntityRegistrar {
      public:
        std::string __name;
        DerivedEntityRegistrar( char* nm ) {
            gm_log("initializing derived object type " + std::string(nm) + " with id ENTITY(" + v_str(Entity<Derived>::id) + ")");
            this->__name = nm;
            Entity<Derived> eclass_1;
            Derived eclass;

            entity_dictionary[ eclass_1.get_serialization_id() ] = std::string(nm);
        };
    };
/// Registers an entity type.
/// Must be called for all new Derived Entity types.
/// also see GMENG_ENTITY_SET_ID(0).
///
/// If you do not register entity types, save files will seg-fault
/// when loading due to no initial registry to the factory having been
/// made. This ensures the factory has an overload for the Derived Class.
#define REGISTER_ENTITY_TYPE( eclass ) \
    static DerivedEntityRegistrar<eclass> derivatory__##eclass( #eclass ); \
    static std::string __derivatory__name__##eclass = derivatory__##eclass.__name; \
    static int __entity_derivatory__id_incr_##eclass = EntityBase::incr_id()

/// ABSOLUTELY DO NOT USE THIS FOR YOUR ENTITIES I DO NOT KNOW ENOUGH
/// OF THE ENGLISH LANGUAGE TO PROPERLY EXPLAIN HOW DAMAGING THIS WILL BE
///
/// JUST USE REGISTER_ENTITY_TYPE. IT IS FOR USERS. IT IS FOR THE EXTERNAL
/// USAGE. NOT THIS. THIS WILL FUCK YOU. AND GIVE YOU STDS.
///
/// GMENG_INTERNAL_ENTITY DOES NOT MAKE YOUR ENTITIES COOL. IT MAKES THEM
/// UNUSABLE BECAUSE THE INTERNAL INCREMENTER HAS GONE OFF COURSE AND NOW
/// YOUR ENTITIES HAVE WRONG ID's ASSIGNED TO THEM
///
/// GOOD LUCK CASTING TO VOID* AND REFACTORING FROM EntityBase
///
/// FOR INTERNAL USAGE:
///    class_def_ending_token {}; GMENG_INTERNAL_ENTITY( EntityName ); REGISTER_ENTITY_TYPE( EntityName );
#define GMENG_INTERNAL_ENTITY( eclass ) \
    template<>      \
    inline const int Entity<eclass>::id = EntityBase::id_counter; \
    inline const int __internal_entity_derivatory_id_increment__##eclass = EntityBase::incr_id()

/// PREVIOUSLY INTERNAL UTILITY THAT I REALISED WAS GOOD FOR NORMAL USERS
/// ---------------------------------------------------------------------
/// PURPOSE: sets internal derivatory_entity_id to a static number. this
///          is good if you ever have to reorder your Entity types for
///          the entire duration of your game's development phase.
///
/// REASON:  extends REGISTER_ENTITY_TYPE(). really good practice since
///          entities are really hard to serialize by themselves because
///          there are different Entity classes with different variables
///          that needs to be saved to a level file. Gmeng does this by
///          incrementing an internal derived_factory_id_counter but this
///          will cause errors if Entity classes initialization order is
///          misaligned / the order is changed. While this is savable by
///          just reordering the Entity initialization to the previous
///          state, it's better to use GMENG_ENTITY_SET_ID( unique_id )
///          to differentiate entity derivatory types. It was a dumb
///          decision in Gmeng 11.0.0 to decide based on id and not typename,
///          but this is a good solution.
///
/// USAGE:   before calling REGISTER_ENTITY_TYPE( DerivedEntityClassName ),
///          call GMENG_ENTITY_SET_ID( DerivedEntityClassName, unique_id ).
///          the id passed into the GMENG_ENTITY_SET_ID must be unique and
///          *NOT RANDOM*. If it's random, your engine build will not be able
///          to load entities from save files and your save file will be
///          rendered useless. set them by hand. don't automate the ids if
///          you use this macro, as it's whole point is manually assigning ids.
///          if you don't want to manually assign ids, REGISTER_ENTITY_TYPE will
///          assign one, but you will have to keep your entity class definition
///          order the same and never add in between and before your previous
///          classes, only after.
/// ---------------------------------------------------------------------
/// FOR INTERNAL USAGE (ONLY GMENG DEVELOPERS *NOT* USERS):
///          VALUES LOWER THAN 0 CAN BE SET TO RESERVE ENTITY IDs FOR INTERNAL
///          ENTITIES. THIS WILL ALLOW FOR SERIALIZATION TO NOT GO CRAZY WHEN
///          NEW GMENG SOURCE ENTITIES ARE ADDED OR THE ORDER OF ENTITIES CHANGE.
/// ---------------------------------------------------------------------
#define GMENG_ENTITY_SET_ID( eclass, ___id ) \
    template<> \
    inline const int Entity<eclass>::id = ___id;

    /// Player Entity object.
    /// Stable for every gmeng instance.
    ///
    /// Extend this class to add different functionality.
    class Player : public Entity<Player> {
      public:
        int interaction_proximity = 10;
        Renderer::drawpoint position = { 0, 0 };

    }; /* do not question why this is not set as an internal entity.
          do even more not set it as an internal entity. just don't.*/
       REGISTER_ENTITY_TYPE( Player );

    /// todo: documentation
    struct LightingState {
        color32_t surface_color;
        color32_t light_color;
        float r_factor;
        float g_factor;
        float b_factor;
        float modulation_factor;

        int brightness;
    };

    /// Light Source Entity object. Used for
    /// illumination in levels. add as an entity
    /// to change brightness & color in your level.
    ///
    /// Requires your level camera to have modifier
    /// 'lighting' to be set as '(int)1'.
    ///
    /// LightSource objects have a size of 1x1 units.
    class LightSource : public Entity<LightSource> {
      public:

#define LIGHTSOURCE_TEXTURE_OBJECT texture{ Blob { 0, 0, {} }, true, "LIGHT_SOURCE" };

        /// LightSource interactions do not happen.
        /// LightSources are not interactable entites,
        /// do not use this variable. Will always be -1.
        int interaction_proximity = -1;
        /// unused : lightsource does not have a texture.
        texture sprite = LIGHTSOURCE_TEXTURE_OBJECT;
        /// color of the light source, changing it
        /// will modulate the units within reach
        /// of the source.
        color32_t color = { 255, 255, 255 };
        /// intensity of the light source, maxes
        /// at 1. for global illumination disable
        /// lighting completely. will turn camera to
        /// full brightness, and disable shading.
        ///
        /// intensity is scaled unit wise and decrementally.
        /// if intensity is 10 at position { 0, 0 },
        /// intensity will be 9 at positions { 1, 0 },
        /// { 0, 1 } and { 0, 1 }; etc.
        int intensity = 15;
        /// Max brightness of the LightSource.
        /// while intensity can be scaled infinitely,
        /// if a max brightness is set then a
        /// Gmeng::Camera::calculate_illumination function call
        /// will not scale higher than the max_brightness value.
        ///
        /// '10' (default) is the max value until colors are
        /// brightened & intensified above their regular values.
        int max_brightness = 10;
        /// Whether the light source is cached.
        /// This is for optimized rendering, as if a light source is cached,
        /// unless the light source was changed the cached data is reusable.
        bool cached = false;
        /// cached minimum brightness value of the light source.
        /// used for rendering in get_renderscale() for cache validation
        int cached_min_brightness = -1;
        /// cached position of the lightsource, for automatic
        /// cache validation and updating if the lightsource
        /// moves, the minimum brightness property changes, etc.
        /// used for rendering in get_renderscale() for cache validation
        Renderer::drawpoint cached_position = { -1, -1 };
        /// cached intensity of the lightsource, for automatic
        /// cache validation and updating if the lightsource's
        /// intensity changes from the inspector/commandline.
        int cached_intensity = -1;
        /// cached color of the lightsource, for automatic
        /// cache validation and updating if the lightsource
        /// moves, the minimum brightness property changes, etc.
        /// used for rendering in get_renderscale() for cache validation
        color32_t cached_color = { 255, 255, 255 };
        /// validates the LightSource's cache by comparing the values to
        /// the object's non-cached properties. Such as:
        ///
        /// this::cached_position COMPARE this::position,
        /// this::cached_min_brightness COMPARE @param min_brightness
        inline void validate_cache( int min_brightness ) {
            /// can't be invalid if you're not cached!
            if ( !this->cached ) return;
            /// check minimum brightness
            if ( min_brightness != cached_min_brightness ) this->cached = false;
            /// check position
            if ( this->cached_position != this->position ) this->cached = false;
            /// check intensity
            if ( this->cached_intensity != this->intensity ) this->cached = false;
            /// check cached color
            if ( this->cached_color != this->color ) this->cached = false;
        };
        /// custom serialization function for
        /// LightSource objects since they contain
        /// variables and parameters the base Entity
        /// type doesn't contain.
        inline void serialize( std::ostream& out ) const override {
            this->default_serialize( out ); /// serialize the default info of the Entity

            out.write(reinterpret_cast<const char*>(&this->intensity), sizeof(intensity));
            out.write(reinterpret_cast<const char*>(&this->max_brightness), sizeof(max_brightness));
            out.write(reinterpret_cast<const char*>(&this->color), sizeof(this->color));

            out.write(reinterpret_cast<const char*>(&position.x), sizeof(position.x));
            out.write(reinterpret_cast<const char*>(&position.y), sizeof(position.y));
        };

        /// custom deserialization function for
        /// LightSource objects since they contain
        /// variables and parameters the base Entity
        /// type doesn't contain.
        inline void deserialize( std::istream& in ) override {
            this->default_deserialize( in ); /// serialize the default info of the Entity

            /// ensure type name
            if ( this->sprite.name != "LIGHT_SOURCE" ) this->sprite.name = "LIGHT_SOURCE";

            in.read(reinterpret_cast<char*>(&this->intensity), sizeof(intensity));
            in.read(reinterpret_cast<char*>(&this->max_brightness), sizeof(max_brightness));
            in.read(reinterpret_cast<char*>(&this->color), sizeof(this->color));

            in.read(reinterpret_cast<char*>(&position.x), sizeof(position.x));
            in.read(reinterpret_cast<char*>(&position.y), sizeof(position.y));
        };
    }; /* LightSource is set to -1 to have a fixed entity id regardless of external entity types.
        * Also, it is not set as a GMENG_INTERNAL_ENTITY since LightSources are not 'entities', but
        * entity-type objects that should not be interactable but are dynamic in design.            */
       GMENG_ENTITY_SET_ID( LightSource, -1 ); REGISTER_ENTITY_TYPE( LightSource );

    /// For versions 12.0.0 and later : SERIALIZED_ENGINE_INFO is an entity
    /// that contains engine info about version specifics and other data
    /// that may vary how the serialization/deserialization is handled
    class SERIALIZED_ENGINE_INFO : public Entity<SERIALIZED_ENGINE_INFO> {
      public:
        std::string build = GMENG_BUILD_NO; std::string version = Gmeng::version;

        texture sprite = { Blob(0, 0, {}), true, "ENGINE_INFO" };

        bool active = false;

        inline void serialize( std::ostream& out ) const override {
            this->default_serialize( out );

            //if (this->build != GMENG_BUILD_NO || this->version != Gmeng::version) {
            //    this->version = Gmeng::version; this->build = std::string(GMENG_BUILD_NO);
            //};

            uint32_t build_len = static_cast<uint32_t>(build.size());
            uint32_t version_len = static_cast<uint32_t>(version.size());

            out.write(reinterpret_cast<const char*>(&build_len), sizeof(build_len));
            out.write( build.data(), build_len );

            out.write(reinterpret_cast<const char*>(&version_len), sizeof(version_len));
            out.write( this->version.data(), version_len );
        };

        inline void deserialize( std::istream& in ) override {
            this->default_deserialize( in );

            uint32_t build_len, version_len;
            in.read(reinterpret_cast<char*>(&build_len), sizeof(build_len));
            build.clear(); build.resize(build_len);
            in.read( &build[0], build_len );

            in.read(reinterpret_cast<char*>(&version_len), sizeof(version_len));
            this->version.clear(); this->version.resize(version_len);
            in.read( &this->version[0], version_len );
        };

    }; GMENG_ENTITY_SET_ID( SERIALIZED_ENGINE_INFO, -2 ); REGISTER_ENTITY_TYPE( SERIALIZED_ENGINE_INFO );

    /// Interactable Entity.
    /// Set the proximity for the proximity limit of the interaction.
    /// set the interaction tag for the interaction callback.
    class Interaction : public Entity<Interaction> {
      public:
        /// the tag of the interaction. When this entity is interacted with,
        /// the callback from Gmeng::Level::callbacks that hold this tag will
        /// be called with Entity* and EventLoop* parameters.
        std::string tag = "unknown_interaction";

        inline void serialize( std::ostream& out ) const override {
            this->default_serialize( out );

            std::size_t tag_length = tag.size();
            out.write( reinterpret_cast<const char*>( tag_length ), sizeof( tag_length ) );
            out.write( this->tag.c_str(), tag_length );

        };

        inline void deserialize( std::istream& in ) override {
            this->default_deserialize( in );

            std::size_t tag_length;
            in.read( reinterpret_cast<char*>(&tag_length), sizeof ( tag_length ) );
            in.read( &this->tag[0], tag_length );
        };

        inline void interact( Entity_Interaction inter, Level* lvl ) override {
            gm_log("interaction entity (tagged " + this->tag+ ") with id "$(this->entity_id)" interacted with by "$(inter.interacted_by->entity_id)". type: "$(inter.type)"");
        };
    }; GMENG_ENTITY_SET_ID( Interaction, -3 ); REGISTER_ENTITY_TYPE( Interaction );


    class Animated_Entity : public Gmeng::Entity<Animated_Entity> {
      using texture = Gmeng::texture;
      public:
        std::vector<texture> anim_textures = {};

        bool animation_enabled = false;
        bool always_animate = false;
        int animation_step = 0;

        bool init = false;
        uint64_t frame_index = 0;

        Gmeng::Renderer::drawpoint cached_pos = this->position;


        void ae_init() {
            if ( this->init ) return;
            this->init = true;
            this->modifiers.sort();

            if ( !this->modifiers.has_value("fps") )
                this->modifiers.add_value("fps", 60);
        };

        /// Animation function. By default, this checks the cached
        /// position of the entity and if it has moved updates the
        /// current sprite the next animation step. Basically a
        /// move/walk animation. Override this to change functionality.
        ///
        /// In version 14.0.0, this will swap to a callback function instead
        /// so not all animations require their seperate entity types.
        inline void animate(Gmeng::EventLoop* ev, int FPS = 60) override {
            if ( this->anim_textures.size() < 1 ) return;
            if ( !this->init ) ae_init();


            if ( (animation_enabled && this->cached_pos != this->position) || this->always_animate ) {

                this->cached_pos = this->position;
                int entity_fps = this->modifiers.get_value("fps");


                this->sprite = anim_textures.at( animation_step );
                /// frame_index = incremented every frame
                /// FIXED_UPDATE_FPS = 60
                /// ENTITY_FPS = 30
                ///
                /// value is if FRAME_INDEX % ( FIXED_UPDATE_FPS / ENTITY_FPS ) == 0
                if ( frame_index % ( FPS / entity_fps ) == 0 ) animation_step++;
                if ( animation_step >= anim_textures.size() ) animation_step = 0;
            } else this->sprite = anim_textures.at(animation_step );
            frame_index++;
        };

        inline texture& draw( Gmeng::Renderer_Type ) override {

            if (this->cached_pos != this->position || this->always_animate) {
                this->animation_enabled = true;
            };

            this->cached_pos = this->position;
            return this->sprite;
        };

        inline void serialize( std::ostream& out ) const override {
            this->default_serialize( out );

            out.write( reinterpret_cast<const char*>( &always_animate ), sizeof( always_animate ) );

            std::size_t texture_size = this->anim_textures.size();
            out.write( reinterpret_cast<const char*>( &texture_size ), sizeof( texture_size ) );
            for ( const auto& texture : this->anim_textures ) {
                serialize_texture( texture, out );
            };
        };

        inline void deserialize( std::istream& in ) override {
            this->default_deserialize( in );

            in.read( reinterpret_cast<char*>(&always_animate), sizeof( always_animate ) );

            std::size_t texture_size;
            in.read( reinterpret_cast<char*>(&texture_size), sizeof( texture_size ) );
            this->anim_textures.resize( texture_size );

            for ( auto& texture : this->anim_textures ) {
                deserialize_texture( texture, in );
            };
        };
    }; GMENG_ENTITY_SET_ID( Animated_Entity, -4 ); REGISTER_ENTITY_TYPE( Animated_Entity );


    /// ENDS ENTITY TYPES:
    ///     GMENG_INTERNAL_ENTITY will place entities at the end of the entity derived factory list.
    ///     So, GMENG_ENTITYTYPES_END::id is a way to check for Entity id overflow.
    class GMENG_ENTITYTYPES_END : public Entity<GMENG_ENTITYTYPES_END> {
    }; GMENG_INTERNAL_ENTITY( GMENG_ENTITYTYPES_END ); REGISTER_ENTITY_TYPE( GMENG_ENTITYTYPES_END );
};


#ifndef GMENG_NO_SOL

#include "../utils/scripts.h"

static int __gmeng_define_lua_entity_method = ([]() -> int {
    using namespace Gmeng;
    using namespace Scripts;

    gmeng_lua_namespace.new_usertype<EntityBase>("Entity",
        "animate", &EntityBase::animate,
        "get_serialization_id", &EntityBase::get_serialization_id,

        "active", &EntityBase::active,
        "cached", &EntityBase::cached,
        "id", &EntityBase::entity_id,
        "modifiers", &EntityBase::modifiers,
        "sprite", &EntityBase::sprite,
        "position", &EntityBase::position
    );
    return 0;
})();

#endif
