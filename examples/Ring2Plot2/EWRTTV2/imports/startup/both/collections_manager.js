import { Mongo } from 'meteor/mongo';

const collections = {};

export const get_collection = name => {
    if (!collections[name]) {
        collections[name] = new Mongo.Collection(name);
    }  
    return collections[name];
}