import { HartDeviceDto } from './hart-device';

export interface HartVarsDto {
    loopCurrent: {
        units: string
        value: number
    }
    pv: {
        units: string
        value: number
    }
    sv?: {
        units: string
        value: number
    }
    tv?: {
        units: string
        value: number
    }
    qv?: {
        units: string
        value: number
    }
}

export interface HartVar {
    value: number,
    units: string,
    lastUpdated: Date
}

export interface HartVars {
    loopCurrent: HartVar
    pv:  HartVar
    sv?: HartVar
    tv?: HartVar
    qv?: HartVar
}

export interface HartDeviceVarsListDto {
    devices: HartDeviceDto[]
}